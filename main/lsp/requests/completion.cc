#include "main/lsp/requests/completion.h"
#include "absl/algorithm/container.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "ast/treemap/treemap.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LocalVarFinder.h"
#include "main/lsp/NextMethodFinder.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

struct RubyKeyword {
    const string keyword;
    const string documentation;
    const optional<string> snippet;
    const optional<string> detail;

    RubyKeyword(string keyword, string documentation, optional<string> snippet = nullopt,
                optional<string> detail = nullopt)
        : keyword(move(keyword)), documentation(move(documentation)), snippet(move(snippet)), detail(move(detail)){};
};

// Taken from https://docs.ruby-lang.org/en/2.6.0/keywords_rdoc.html
// We might want to put this somewhere shareable if there are more places that want to use it.
//
// VS Code snippet syntax is in general smarter than LSP snippet syntax.
// Specifically, VS Code will intelligently insert the correct indentation after newlines.
const RubyKeyword rubyKeywords[] = {
    {"BEGIN", "Runs before any other code in the current file."},
    {"END", "Runs after any other code in the current file."},
    {"__ENCODING__", "The script encoding of the current file."},
    {"__FILE__", "The path to the current file."},
    {"__LINE__", "The line number of this keyword in the current file."},
    {"alias", "Creates an alias between two methods (and other things).", "alias ${1:_new} ${2:_existing}$0"},
    {"and", "Short-circuit Boolean and with lower precedence than &&"},
    {"begin", "Starts an exception handling block.", "begin\n  $0\nend"},
    {"break", "Leaves a block early."},
    {"case", "Starts a case expression.", "case ${1:expr}\nwhen ${2:expr}\n  $0\nelse\nend", "case/when/else/end"},
    {"class", "Creates or opens a class.", "class ${1:ClassName}\n  $0\nend", "New class"},
    {"def", "Defines a method.", "def ${1:method_name}($2)\n  $0\nend", "New method"},
    {"defined?", "Returns a string describing its argument.", "defined?(${1:Constant})$0"},
    {"do", "Starts a block.", "do\n  $0\nend"},
    {"else", "The unhandled condition in case, if and unless expressions."},
    {"elsif", "An alternate condition for an if expression.", "elsif ${1:expr}$0"},
    {"end",
     "The end of a syntax block. Used by classes, modules, methods, exception handling and control expressions."},
    {"ensure", "Starts a section of code that is always run when an exception is raised."},
    {"false", "Boolean false."},
    {"for", "A loop that is similar to using the each method."},
    {"if", "Used for if and modifier if expressions.", "if ${1:expr}\n  $0\nend", "if/end"},
    {"in", "Used to separate the iterable object and iterator variable in a for loop."},
    {"module", "Creates or opens a module.", "module ${1:ModuleName}\n  $0\nend", "New module"},
    {"next", "Skips the rest of the block."},
    {"nil", "A false value usually indicating “no value” or “unknown”."},
    {"not", "Inverts the following boolean expression. Has a lower precedence than !"},
    {"or", "Boolean or with lower precedence than ||"},
    {"redo", "Restarts execution in the current block."},
    // Would really like to dedent the line too...
    {"rescue", "Starts an exception section of code in a begin block.", "rescue ${1:MyException} => ${2:ex}\n$0"},
    {"retry", "Retries an exception block."},
    {"return", "Exits a method."},
    {"self", "The object the current method is attached to."},
    {"super", "Calls the current method in a superclass."},
    {"then", "Indicates the end of conditional blocks in control structures."},
    {"true", "Boolean true."},
    // This is also defined on Kernel
    // {"undef", "Prevents a class or module from responding to a method call."},
    {"unless", "Used for unless and modifier unless expressions.", "unless ${1:expr}\n  $0\nend", "unless/end"},
    {"until", "Creates a loop that executes until the condition is true.", "until ${1:expr}\n  $0\nend", "until/end"},
    // Would really like to dedent the line too...
    {"when", "A condition in a case expression.", "when ${1:expr}$0"},
    {"while", "Creates a loop that executes while the condition is true.", "while ${1:expr}\n  $0\nend", "while/end"},
    {"yield", "Starts execution of the block sent to the current method."},
};

vector<core::ClassOrModuleRef> ancestorsImpl(const core::GlobalState &gs, core::ClassOrModuleRef sym,
                                             vector<core::ClassOrModuleRef> &&acc) {
    // The implementation here is similar to Symbols::derivesFrom.
    ENFORCE(sym.data(gs)->isClassOrModuleLinearizationComputed());
    acc.emplace_back(sym);

    for (auto mixin : sym.data(gs)->mixins()) {
        acc.emplace_back(mixin);
    }

    if (sym.data(gs)->superClass().exists()) {
        return ancestorsImpl(gs, sym.data(gs)->superClass(), move(acc));
    } else {
        return move(acc);
    }
}

// Basically the same as Module#ancestors from Ruby--but don't depend on it being exactly equal.
// For us, it's just something that's vaguely ordered from "most specific" to "least specific" ancestor.
vector<core::ClassOrModuleRef> ancestors(const core::GlobalState &gs, core::ClassOrModuleRef receiver) {
    return ancestorsImpl(gs, receiver, vector<core::ClassOrModuleRef>{});
}

struct SimilarMethod final {
    int depth;
    core::ClassOrModuleRef receiver;
    core::MethodRef method;

    // Populated later
    core::TypePtr receiverType = nullptr;
    shared_ptr<core::TypeConstraint> constr = nullptr;
};

bool hasAngleBrackets(string_view haystack) {
    return absl::c_any_of(haystack, [](char c) { return c == '<' || c == '>'; });
}

// Note: this will always return `true` for `pattern == ""`.
bool hasSimilarName(const core::GlobalState &gs, core::NameRef name, string_view pattern) {
    string_view view = name.shortName(gs);
    auto fnd = view.find(pattern);
    return fnd != string_view::npos;
}

using SimilarMethodsByName = UnorderedMap<core::NameRef, vector<SimilarMethod>>;

// First of pair is "found at this depth in the ancestor hierarchy"
// Second of pair is method symbol found at that depth, with name similar to prefix.
SimilarMethodsByName similarMethodsForClass(const core::GlobalState &gs, core::ClassOrModuleRef receiver,
                                            string_view prefix) {
    auto result = SimilarMethodsByName{};

    int depth = -1;
    for (auto ancestor : ancestors(gs, receiver)) {
        depth++;
        for (auto [memberName, memberSymbol] : ancestor.data(gs)->members()) {
            if (!memberSymbol.isMethod()) {
                continue;
            }
            if (hasAngleBrackets(memberName.shortName(gs))) {
                // Gets rid of methods like `<test_foo bar>` generated by our DSL passes
                continue;
            }

            if (hasSimilarName(gs, memberName, prefix)) {
                // Creates the the list if it does not exist
                result[memberName].emplace_back(SimilarMethod{depth, receiver, memberSymbol.asMethodRef()});
            }
        }
    }

    return result;
}

// Unconditionally creates an intersection of the methods
// (for both union and intersection types, it's only valid to call a method by name if it exists on all components)
SimilarMethodsByName mergeSimilarMethods(SimilarMethodsByName left, SimilarMethodsByName right) {
    auto result = SimilarMethodsByName{};

    for (auto &[methodName, leftSimilarMethods] : left) {
        if (right.contains(methodName)) {
            for (auto &similarMethod : leftSimilarMethods) {
                result[methodName].emplace_back(similarMethod);
            }
            for (auto &similarMethod : right[methodName]) {
                result[methodName].emplace_back(similarMethod);
            }
        }
    }
    return result;
}

SimilarMethodsByName similarMethodsForReceiver(const core::GlobalState &gs, const core::TypePtr &receiver,
                                               string_view prefix) {
    auto result = SimilarMethodsByName{};

    typecase(
        receiver, [&](const core::ClassType &type) { result = similarMethodsForClass(gs, type.symbol, prefix); },
        [&](const core::AppliedType &type) { result = similarMethodsForClass(gs, type.klass, prefix); },
        [&](const core::AndType &type) {
            result = mergeSimilarMethods(similarMethodsForReceiver(gs, type.left, prefix),
                                         similarMethodsForReceiver(gs, type.right, prefix));
        },
        [&](const core::TypePtr &type) {
            if (is_proxy_type(receiver)) {
                result = similarMethodsForReceiver(gs, receiver.underlying(gs), prefix);
            }
        });

    return result;
}

// Walk a core::DispatchResult to find methods similar to `prefix` on any of its DispatchComponents' receivers.
SimilarMethodsByName allSimilarMethods(const core::GlobalState &gs, core::DispatchResult &dispatchResult,
                                       string_view prefix) {
    auto result = similarMethodsForReceiver(gs, dispatchResult.main.receiver, prefix);

    for (auto &[methodName, similarMethods] : result) {
        for (auto &similarMethod : similarMethods) {
            ENFORCE(similarMethod.receiverType == nullptr, "About to overwrite non-null receiverType");
            similarMethod.receiverType = dispatchResult.main.receiver;
        }
    }

    if (dispatchResult.secondary != nullptr) {
        // Right now we completely ignore the secondaryKind (either AND or OR), and always intersect.
        // (See comment above mergeSimilarMethods)
        result = mergeSimilarMethods(result, allSimilarMethods(gs, *dispatchResult.secondary, prefix));
    }

    return result;
}

vector<RubyKeyword> allSimilarKeywords(string_view prefix) {
    ENFORCE(absl::c_is_sorted(rubyKeywords, [](auto &left, auto &right) { return left.keyword < right.keyword; }),
            "rubyKeywords is not sorted by keyword; completion results will be out of order");

    if (prefix == "") {
        // Since we suggest keyword snippets first, they're just noise when the prefix is empty
        return {};
    }

    auto result = vector<RubyKeyword>{};
    for (const auto &rubyKeyword : rubyKeywords) {
        if (absl::StartsWith(rubyKeyword.keyword, prefix)) {
            result.emplace_back(rubyKeyword);
        }
    }

    // The result is trivially sorted because we walked rubyKeywords (which is sorted) in order.
    return result;
}

vector<core::NameRef> allSimilarLocalNames(const core::GlobalState &gs, const vector<core::NameRef> &locals,
                                           string_view prefix) {
    auto result = vector<core::NameRef>{};
    for (const auto &local : locals) {
        if (hasAngleBrackets(local.shortName(gs))) {
            // Gets rid of locals like <blk>
            continue;
        }

        if (hasSimilarName(gs, local, prefix)) {
            result.emplace_back(local);
        }
    }

    return result;
}

string methodSnippet(const core::GlobalState &gs, core::DispatchResult &dispatchResult, core::MethodRef method,
                     const core::TypePtr &receiverType, const core::TypeConstraint *constraint, uint16_t totalArgs) {
    fmt::memory_buffer result;
    auto shortName = method.data(gs)->name.shortName(gs);
    auto isSetter = method.data(gs)->name.isSetter(gs);
    if (isSetter) {
        fmt::format_to(std::back_inserter(result), "{}", string_view(shortName.data(), shortName.size() - 1));
    } else {
        fmt::format_to(std::back_inserter(result), "{}", shortName);
    }
    auto nextTabstop = 1;

    /* If we are completing an existing send that either has some arguments
     * or a block specified already then simply complete the method name
     * since the rest is likely useless
     */
    if (totalArgs > 0 || dispatchResult.main.blockReturnType != nullptr) {
        fmt::format_to(std::back_inserter(result), "${{0}}");
        return to_string(result);
    }

    if (isSetter) {
        fmt::format_to(std::back_inserter(result), " = ${{0}}");
        return to_string(result);
    }

    vector<string> typeAndArgNames;
    for (auto &argSym : method.data(gs)->arguments) {
        fmt::memory_buffer argBuf;
        if (argSym.flags.isBlock) {
            // Blocks are handled below
            continue;
        }
        if (argSym.flags.isDefault) {
            continue;
        }
        if (argSym.flags.isRepeated) {
            continue;
        }
        if (argSym.flags.isKeyword) {
            fmt::format_to(std::back_inserter(argBuf), "{}: ", argSym.name.shortName(gs));
        }
        if (argSym.type) {
            auto resultType = getResultType(gs, argSym.type, method, receiverType, constraint).show(gs);
            fmt::format_to(std::back_inserter(argBuf), "${{{}:{}}}", nextTabstop++, resultType);
        } else {
            fmt::format_to(std::back_inserter(argBuf), "${{{}}}", nextTabstop++);
        }
        typeAndArgNames.emplace_back(to_string(argBuf));
    }

    if (!typeAndArgNames.empty()) {
        fmt::format_to(std::back_inserter(result), "({})", fmt::join(typeAndArgNames, ", "));
    }

    ENFORCE(!method.data(gs)->arguments.empty());
    auto &blkArg = method.data(gs)->arguments.back();
    ENFORCE(blkArg.flags.isBlock);

    auto hasBlockType = blkArg.type != nullptr && !blkArg.type.isUntyped();
    if (hasBlockType && !core::Types::isSubType(gs, core::Types::nilClass(), blkArg.type)) {
        string blkArgs;
        if (auto *appliedType = core::cast_type<core::AppliedType>(blkArg.type)) {
            if (appliedType->targs.size() >= 2) {
                // The first element in targs is the return type.
                auto targs_it = appliedType->targs.begin();
                targs_it++;
                blkArgs = fmt::format(" |{}|", fmt::map_join(targs_it, appliedType->targs.end(), ", ", [&](auto targ) {
                                          auto resultType = getResultType(gs, targ, method, receiverType, constraint);
                                          return fmt::format("${{{}:{}}}", nextTabstop++, resultType.show(gs));
                                      }));
            }
        }

        fmt::format_to(std::back_inserter(result), " do{}\n  ${{{}}}\nend", blkArgs, nextTabstop++);
    }

    fmt::format_to(std::back_inserter(result), "${{0}}");
    return to_string(result);
}

// This is an approximation. It takes advantage of the fact that nearly all of the time,
// the prefix being used to suggest completion items actually accurred in the source text
// of the file, immediately before the queryLoc.
//
// This is somewhat brittle, but has worked well so far.
unique_ptr<Range> replacementRangeForQuery(const core::GlobalState &gs, core::Loc queryLoc, string_view prefix) {
    auto queryStart = queryLoc.beginPos();
    uint32_t prefixSize = prefix.size();
    auto replacementLoc = core::Loc{queryLoc.file(), queryStart - prefixSize, queryStart};
    // Sometimes Range::fromLoc returns nullptr (commonly when running under a fuzzer which disables certain loc info).
    return Range::fromLoc(gs, replacementLoc);
}

string formatSortIndex(size_t sortIdx) {
    // Completion items are sorted by sortText if present, or label if not. We unconditionally use an index to sort.
    // If we ever have 100,000+ items in the completion list, we'll need to bump the padding here.
    return fmt::format("{:06d}", sortIdx);
}

unique_ptr<CompletionItem> getCompletionItemForKeyword(const core::GlobalState &gs, const LSPConfiguration &config,
                                                       const RubyKeyword &rubyKeyword, const core::Loc queryLoc,
                                                       string_view prefix, size_t sortIdx) {
    auto supportSnippets = config.getClientConfig().clientCompletionItemSnippetSupport;
    auto markupKind = config.getClientConfig().clientCompletionItemMarkupKind;
    auto item = make_unique<CompletionItem>(rubyKeyword.keyword);
    item->sortText = formatSortIndex(sortIdx);

    string replacementText;
    if (rubyKeyword.snippet.has_value() && supportSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
        item->kind = CompletionItemKind::Snippet;
        replacementText = rubyKeyword.snippet.value();
    } else {
        item->insertTextFormat = InsertTextFormat::PlainText;
        item->kind = CompletionItemKind::Keyword;
        replacementText = rubyKeyword.keyword;
    }

    if (auto replacementRange = replacementRangeForQuery(gs, queryLoc, prefix)) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
    } else {
        item->insertText = replacementText;
    }

    if (rubyKeyword.detail.has_value()) {
        item->detail = fmt::format("(sorbet) {}", rubyKeyword.detail.value());
    } else if (item->kind == CompletionItemKind::Snippet) {
        item->detail = fmt::format("(sorbet) Snippet: {}", rubyKeyword.keyword);
    } else {
        item->detail = fmt::format("(sorbet) Ruby keyword: {}", rubyKeyword.keyword);
    }

    if (rubyKeyword.snippet.has_value()) {
        item->documentation = formatRubyMarkup(markupKind, rubyKeyword.snippet.value(), rubyKeyword.documentation);
    } else {
        item->documentation = rubyKeyword.documentation;
    }

    return item;
}

unique_ptr<CompletionItem> getCompletionItemForConstant(const core::GlobalState &gs, const LSPConfiguration &config,
                                                        const core::SymbolRef maybeAlias, const core::Loc queryLoc,
                                                        string_view prefix, size_t sortIdx) {
    ENFORCE(maybeAlias.exists());

    auto clientConfig = config.getClientConfig();
    auto supportsSnippets = clientConfig.clientCompletionItemSnippetSupport;
    auto markupKind = clientConfig.clientCompletionItemMarkupKind;

    auto label = string(maybeAlias.name(gs).shortName(gs));

    // Intuition for when to use maybeAlias vs what: if it needs to know the original name: maybeAlias.
    // If it needs to know the types / arity: what. Default to `what` if you don't know.
    auto what = maybeAlias.dealias(gs);

    auto item = make_unique<CompletionItem>(label);

    item->sortText = formatSortIndex(sortIdx);

    if (what.isClassOrModule()) {
        auto whatKlass = what.asClassOrModuleRef();
        if (whatKlass.data(gs)->isClassOrModuleClass()) {
            if (whatKlass.data(gs)->derivesFrom(gs, core::Symbols::T_Enum())) {
                item->kind = CompletionItemKind::Enum;
            } else {
                item->kind = CompletionItemKind::Class;
            }
        } else {
            if (whatKlass.data(gs)->isClassOrModuleAbstract() || whatKlass.data(gs)->isClassOrModuleInterface()) {
                item->kind = CompletionItemKind::Interface;
            } else {
                item->kind = CompletionItemKind::Module;
            }
        }
    } else if (what.isTypeMember() || what.isStaticField(gs)) {
        item->kind = CompletionItemKind::Field;
    } else {
        ENFORCE(false, "Unhandled kind of constant in getCompletionItemForConstant");
    }

    item->detail = maybeAlias.show(gs);

    string replacementText;
    if (supportsSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
        replacementText = fmt::format("{}${{0}}", label);
    } else {
        item->insertTextFormat = InsertTextFormat::PlainText;
        replacementText = label;
    }

    if (auto replacementRange = replacementRangeForQuery(gs, queryLoc, prefix)) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
    } else {
        item->insertText = replacementText;
    }

    optional<string> documentation = nullopt;
    auto whatFile = what.loc(gs).file();
    if (whatFile.exists()) {
        documentation = findDocumentation(whatFile.data(gs).source(), what.loc(gs).beginPos());
    }

    auto prettyType = prettyTypeForConstant(gs, what);
    item->documentation = formatRubyMarkup(markupKind, prettyType, documentation);

    return item;
}

unique_ptr<CompletionItem> getCompletionItemForLocalName(const core::GlobalState &gs, const LSPConfiguration &config,
                                                         const core::NameRef local, const core::Loc queryLoc,
                                                         string_view prefix, size_t sortIdx) {
    auto label = string(local.shortName(gs));
    auto item = make_unique<CompletionItem>(label);
    item->sortText = formatSortIndex(sortIdx);
    item->kind = CompletionItemKind::Variable;

    auto replacementText = label;
    if (auto replacementRange = replacementRangeForQuery(gs, queryLoc, prefix)) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
    } else {
        item->insertText = replacementText;
    }
    item->insertTextFormat = InsertTextFormat::PlainText;

    return item;
}

vector<core::NameRef> localNamesForMethod(LSPTypecheckerDelegate &typechecker, const core::MethodRef method,
                                          const core::Loc queryLoc) {
    const auto &gs = typechecker.state();
    auto files = vector<core::FileRef>{};
    for (auto loc : method.data(gs)->locs()) {
        files.emplace_back(loc.file());
    }
    auto resolved = typechecker.getResolved(files);

    // Instantiate localVarFinder outside loop so that result accumualates over every time we TreeMap::apply
    LocalVarFinder localVarFinder(method, queryLoc);
    for (auto &t : resolved) {
        auto ctx = core::Context(gs, core::Symbols::root(), t.file);
        t.tree = ast::TreeMap::apply(ctx, localVarFinder, move(t.tree));
    }

    auto result = localVarFinder.result();
    fast_sort(result, [&gs](const auto &left, const auto &right) {
        // Sort by actual name, not by NameRef id
        if (left != right) {
            return left.shortName(gs) < right.shortName(gs);
        } else {
            return left.rawId() < right.rawId();
        }
    });

    // Dedup
    auto it = unique(result.begin(), result.end());
    result.erase(it, result.end());
    return result;
}

core::MethodRef firstMethodAfterQuery(LSPTypecheckerDelegate &typechecker, const core::Loc queryLoc) {
    const auto &gs = typechecker.state();
    auto files = vector<core::FileRef>{queryLoc.file()};
    auto resolved = typechecker.getResolved(files);

    NextMethodFinder nextMethodFinder(queryLoc);
    for (auto &t : resolved) {
        auto ctx = core::Context(gs, core::Symbols::root(), t.file);
        t.tree = ast::TreeMap::apply(ctx, nextMethodFinder, move(t.tree));
    }

    return nextMethodFinder.result();
}

// This code is ugly but I'm convinced it's because of C++'s baroque string APIs, not for lack
// of trying to make this code prettier. If you're up to the challenge, feel free.
string suggestedSigToSnippet(string_view suggestedSig) {
    auto result = fmt::format("{}${{0}}", suggestedSig);

    auto tabstopId = 1;
    size_t replaceFrom = 0;
    while (true) {
        string needle = "T.untyped";
        replaceFrom = result.find(needle, replaceFrom);
        if (replaceFrom == string::npos) {
            break;
        }

        auto replaceWith = fmt::format("${{{}:T.untyped}}", tabstopId);
        result.replace(replaceFrom, needle.size(), replaceWith);
        tabstopId++;
        replaceFrom += replaceWith.size();
    }

    return result;
}

constexpr string_view suggestSigDocs =
    "Sorbet suggests this signature given the method below. Sorbet's suggested sigs are imperfect. It doesn't always "
    "guess the correct types (or any types at all), but they're usually a good starting point."sv;

unique_ptr<CompletionItem> trySuggestSig(LSPTypecheckerDelegate &typechecker,
                                         const LSPClientConfiguration &clientConfig, core::SymbolRef what,
                                         core::TypePtr receiverType, const core::Loc queryLoc, string_view prefix,
                                         size_t sortIdx) {
    ENFORCE(receiverType != nullptr);

    // Completion with T::Sig::WithoutRuntime.sig / Sorbet::Private::Static.sig won't work because
    // this code path relies on the DispatchResult's receiver type being the `self` of the module
    // where the completion is happening, which isn't true for those two.  Luckily, this won't
    // happen in practice, because SigSuggestion.cc short circuits if the method already has a sig.
    ENFORCE(what == core::Symbols::sig());

    const auto &gs = typechecker.state();
    const auto markupKind = clientConfig.clientCompletionItemMarkupKind;
    const auto supportSnippets = clientConfig.clientCompletionItemSnippetSupport;

    auto targetMethod = firstMethodAfterQuery(typechecker, queryLoc);
    if (!targetMethod.exists()) {
        return nullptr;
    }

    core::ClassOrModuleRef receiverSym;
    if (core::isa_type<core::ClassType>(receiverType)) {
        auto classType = core::cast_type_nonnull<core::ClassType>(receiverType);
        receiverSym = classType.symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(receiverType)) {
        receiverSym = appliedType->klass;
    } else {
        // receiverType is not a simple type. This can happen for any number of strange and uncommon reasons, like:
        // x = T.let(self, T.nilable(T::Sig));  x.sig {void}
        return nullptr;
    }

    if (receiverSym == core::Symbols::rootSingleton()) {
        receiverSym = core::Symbols::Object().data(gs)->lookupSingletonClass(gs);
    }
    auto methodOwner = targetMethod.data(gs)->owner;

    if (!(methodOwner == receiverSym || methodOwner == receiverSym.data(gs)->attachedClass(gs))) {
        // The targetMethod we were going to suggest a sig for is not actually in the same scope as this sig.
        return nullptr;
    }

    auto queryFiles = vector<core::FileRef>{queryLoc.file()};
    auto queryResult = typechecker.query(core::lsp::Query::createSuggestSigQuery(targetMethod), queryFiles);
    if (queryResult.error) {
        return nullptr;
    }

    auto &queryResponses = queryResult.responses;
    if (queryResponses.empty()) {
        return nullptr;
    }

    auto editResponse = queryResponses[0]->isEdit();
    if (editResponse == nullptr) {
        return nullptr;
    }

    auto item = make_unique<CompletionItem>("sig");
    item->kind = CompletionItemKind::Method;
    item->sortText = formatSortIndex(sortIdx);
    item->detail = fmt::format("Suggested sig for {}", targetMethod.data(gs)->name.shortName(gs));

    uint32_t queryStart = queryLoc.beginPos();
    uint32_t prefixSize = prefix.size();
    auto replacementLoc = core::Loc{queryLoc.file(), queryStart - prefixSize, queryStart};
    auto replacementRange = Range::fromLoc(gs, replacementLoc);

    // SigSuggestion.cc computes the replacement text assuming it will be inserted immediately in front of the def,
    // which means it has a newline and indentation at the end of the replacement. We don't need that whitespace
    // because we can just replace the prefix that the user has already started typing.
    auto suggestedSig = absl::StripTrailingAsciiWhitespace(editResponse->replacement);
    string replacementText;
    if (supportSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
        replacementText = suggestedSigToSnippet(suggestedSig);
    } else {
        item->insertTextFormat = InsertTextFormat::PlainText;
        replacementText = suggestedSig;
    }

    if (replacementRange != nullptr) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), string(replacementText));
    } else {
        item->insertText = replacementText;
    }

    item->documentation = formatRubyMarkup(markupKind, suggestedSig, suggestSigDocs);

    return item;
}

bool isSimilarConstant(const core::GlobalState &gs, string_view prefix, core::SymbolRef sym) {
    if (!sym.exists()) {
        return false;
    }

    if (!(sym.isClassOrModule() || sym.isStaticField(gs) || sym.isTypeMember())) {
        return false;
    }

    auto name = sym.name(gs);
    if (name.kind() != core::NameKind::CONSTANT) {
        return false;
    }

    if (name.isTEnumName(gs)) {
        // Every T::Enum value gets a class with the ~same name (see rewriter/TEnum.cc for details).
        // This manifests as showing two completion results when we should only show one, so skip the bad kind.
        return false;
    }

    if (hasAngleBrackets(name.shortName(gs))) {
        // Gets rid of classes like `<Magic>`; they can't be typed by a user anyways.
        return false;
    }

    return hasSimilarName(gs, name, prefix);
}

vector<unique_ptr<CompletionItem>> allSimilarConstantItems(const core::GlobalState &gs, const LSPConfiguration &config,
                                                           string_view prefix,
                                                           core::lsp::ConstantResponse::Scopes &scopes,
                                                           core::Loc queryLoc, size_t initialSortIdx) {
    config.logger->debug("Looking for constant similar to {}", prefix);
    ENFORCE(!scopes.empty());

    vector<unique_ptr<CompletionItem>> items;

    if (scopes.size() == 1 && !scopes[0].exists()) {
        // This happens when there was a contant literal like C::D but `C` itself was stubbed,
        // so we have no idea what `D` is or what its resolution scope is.
        return items;
    }

    for (auto scope : scopes) {
        if (!scope.isClassOrModule()) {
            continue;
        }

        // TODO(jez) This membersStableOrderSlow is the only ordering we have on constant items right now.
        // We should probably at least sort by whether the prefix of the suggested constant matches.
        for (auto [_name, sym] : scope.asClassOrModuleRef().data(gs)->membersStableOrderSlow(gs)) {
            if (isSimilarConstant(gs, prefix, sym)) {
                items.push_back(
                    getCompletionItemForConstant(gs, config, sym, queryLoc, prefix, initialSortIdx + items.size()));
            }
        }
    }

    if (scopes.size() == 1) {
        // If scope is size one, that means we were either given an explicit scope (::A, B::C),
        // or we've been requested to resolve a bare constant at the top level.
        // In either case, we want to skip looking through ancestors and instead suggest constants only on that scope.
        return items;
    }

    int i = -1;
    for (auto ancestor : ancestors(gs, scopes[0].asClassOrModuleRef())) {
        i++;

        if (i == 0) {
            // Skip first ancestor; it already showed up in the search over nesting scope.
            continue;
        }

        for (auto [_name, sym] : ancestor.data(gs)->membersStableOrderSlow(gs)) {
            if (isSimilarConstant(gs, prefix, sym)) {
                items.push_back(
                    getCompletionItemForConstant(gs, config, sym, queryLoc, prefix, initialSortIdx + items.size()));
            }
        }
    }

    return items;
}

} // namespace

CompletionTask::CompletionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CompletionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCompletion), params(move(params)) {}

unique_ptr<CompletionItem>
CompletionTask::getCompletionItemForMethod(LSPTypecheckerDelegate &typechecker, core::DispatchResult &dispatchResult,
                                           core::MethodRef maybeAlias, const core::TypePtr &receiverType,
                                           const core::TypeConstraint *constraint, core::Loc queryLoc,
                                           string_view prefix, size_t sortIdx, uint16_t totalArgs) const {
    const auto &gs = typechecker.state();
    ENFORCE(maybeAlias.exists());
    auto clientConfig = config.getClientConfig();
    auto supportsSnippets = clientConfig.clientCompletionItemSnippetSupport;
    auto markupKind = clientConfig.clientCompletionItemMarkupKind;

    auto label = string(maybeAlias.data(gs)->name.shortName(gs));

    // Intuition for when to use maybeAlias vs what: if it needs to know the original name: maybeAlias.
    // If it needs to know the types / arity: what. Default to `what` if you don't know.
    auto what = maybeAlias.data(gs)->dealiasMethod(gs);

    if (what == core::Symbols::sig()) {
        if (auto item = trySuggestSig(typechecker, clientConfig, what, receiverType, queryLoc, prefix, sortIdx)) {
            return item;
        }
    }

    auto item = make_unique<CompletionItem>(label);

    item->sortText = formatSortIndex(sortIdx);

    item->kind = CompletionItemKind::Method;
    item->detail = maybeAlias.show(gs);

    string replacementText;
    if (supportsSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
        replacementText = methodSnippet(gs, dispatchResult, what, receiverType, constraint, totalArgs);
    } else {
        item->insertTextFormat = InsertTextFormat::PlainText;
        replacementText = label;
    }

    if (auto replacementRange = replacementRangeForQuery(gs, queryLoc, prefix)) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
    } else {
        item->insertText = replacementText;
    }

    optional<string> documentation = nullopt;
    auto whatFile = what.data(gs)->loc().file();
    if (whatFile.exists()) {
        documentation = findDocumentation(whatFile.data(gs).source(), what.data(gs)->loc().beginPos());
    }

    auto prettyType = prettyTypeForMethod(gs, maybeAlias, receiverType, nullptr, constraint);
    item->documentation = formatRubyMarkup(markupKind, prettyType, documentation);

    if (documentation != nullopt && documentation->find("@deprecated") != documentation->npos) {
        item->deprecated = true;
    }

    return item;
}

CompletionTask::MethodSearchParams CompletionTask::methodSearchParamsForEmptyAssign(const core::GlobalState &gs,
                                                                                    core::MethodRef enclosingMethod) {
    auto returnType = core::Types::untypedUntracked();
    auto receiverType = enclosingMethod.data(gs)->owner.data(gs)->externalType(); // self
    auto dispatchMethod = core::MethodRef{};
    size_t totalArgs = 0;
    auto isPrivateOk = true;
    return MethodSearchParams{
        make_shared<core::DispatchResult>(returnType, receiverType, dispatchMethod),
        totalArgs,
        isPrivateOk,
    };
}

// Manually craft a set of SearchParams that correspond to en "empty" cursor position
CompletionTask::SearchParams CompletionTask::searchParamsForEmptyAssign(const core::GlobalState &gs, core::Loc queryLoc,
                                                                        core::MethodRef enclosingMethod,
                                                                        core::lsp::ConstantResponse::Scopes scopes) {
    auto prefix = "";
    // Create a fake DispatchResult to get method results
    auto suggestKeywords = true;
    return SearchParams{
        queryLoc,
        prefix,
        methodSearchParamsForEmptyAssign(gs, enclosingMethod),
        suggestKeywords,
        enclosingMethod, // locals
        core::lsp::ConstantResponse::Scopes{enclosingMethod.data(gs)->owner},
    };
}

vector<unique_ptr<CompletionItem>> CompletionTask::getCompletionItems(LSPTypecheckerDelegate &typechecker,
                                                                      SearchParams &params) {
    const auto &gs = typechecker.state();

    // ----- locals -----

    vector<core::NameRef> similarLocals;
    if (params.enclosingMethod.exists()) {
        auto locals = localNamesForMethod(typechecker, params.enclosingMethod, params.queryLoc);
        similarLocals = allSimilarLocalNames(gs, locals, params.prefix);
    }

    // ----- keywords -----

    auto similarKeywords = params.suggestKeywords ? allSimilarKeywords(params.prefix) : vector<RubyKeyword>{};

    // ----- methods -----

    SimilarMethodsByName similarMethodsByName;
    if (params.forMethods != nullopt) {
        similarMethodsByName = allSimilarMethods(gs, *params.forMethods->dispatchResult, params.prefix);
        for (auto &[methodName, similarMethods] : similarMethodsByName) {
            fast_sort(similarMethods, [&](const auto &left, const auto &right) -> bool {
                if (left.depth != right.depth) {
                    return left.depth < right.depth;
                }

                return left.method.id() < right.method.id();
            });
        }
    }

    auto dedupedSimilarMethods = vector<SimilarMethod>{};
    for (auto &[methodName, similarMethods] : similarMethodsByName) {
        if (methodName.kind() == core::NameKind::UNIQUE &&
            methodName.dataUnique(gs)->uniqueNameKind == core::UniqueNameKind::MangleRename) {
            // It's possible we want to ignore more things here. But note that we *don't* want to ignore all
            // unique names, because we want each overload to show up but those use unique names.
            continue;
        }

        // Since each list is sorted by depth, taking the first elem dedups by depth within each name.
        auto similarMethod = similarMethods[0];

        if (similarMethod.method.data(gs)->flags.isPrivate && !params.forMethods->isPrivateOk) {
            continue;
        }

        dedupedSimilarMethods.emplace_back(similarMethod);
    }

    fast_sort(dedupedSimilarMethods, [&](const auto &left, const auto &right) -> bool {
        if (left.depth != right.depth) {
            return left.depth < right.depth;
        }

        auto leftShortName = left.method.data(gs)->name.shortName(gs);
        auto rightShortName = right.method.data(gs)->name.shortName(gs);
        if (leftShortName != rightShortName) {
            if (absl::StartsWith(leftShortName, params.prefix) && !absl::StartsWith(rightShortName, params.prefix)) {
                return true;
            }
            if (!absl::StartsWith(leftShortName, params.prefix) && absl::StartsWith(rightShortName, params.prefix)) {
                return false;
            }

            return leftShortName < rightShortName;
        }

        return left.method.id() < right.method.id();
    });

    // ----- final sort -----

    // TODO(jez) Do something smarter here than "all keywords then all locals then all methods then all constants"

    vector<unique_ptr<CompletionItem>> items;
    for (auto &similarKeyword : similarKeywords) {
        items.push_back(getCompletionItemForKeyword(gs, this->config, similarKeyword, params.queryLoc, params.prefix,
                                                    items.size()));
    }
    for (auto &similarLocal : similarLocals) {
        items.push_back(getCompletionItemForLocalName(gs, this->config, similarLocal, params.queryLoc, params.prefix,
                                                      items.size()));
    }
    for (auto &similarMethod : dedupedSimilarMethods) {
        // Even though we might have one or more TypeConstraints on the DispatchResult that triggered this completion
        // request, those constraints are the result of solving the current method. These new methods we're about to
        // suggest are their own methods with their own type variables, so it doesn't make sense to use the old
        // constraint for the new methods.
        //
        // What this means in practice is that the prettified `sig` in the completion documentation will show
        // `T.type_parameter(:U)` instead of a solved type.
        auto constr = nullptr;

        items.push_back(getCompletionItemForMethod(
            typechecker, *params.forMethods->dispatchResult, similarMethod.method, similarMethod.receiverType, constr,
            params.queryLoc, params.prefix, items.size(), params.forMethods->totalArgs));
    }

    if (!params.scopes.empty()) {
        auto similarConsts =
            allSimilarConstantItems(gs, this->config, params.prefix, params.scopes, params.queryLoc, items.size());
        move(similarConsts.begin(), similarConsts.end(), back_inserter(items));
    }

    return items;
}

unique_ptr<ResponseMessage> CompletionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCompletion);
    auto emptyResult = make_unique<CompletionList>(false, vector<unique_ptr<CompletionItem>>{});

    const auto &gs = typechecker.state();
    auto uri = params->textDocument->uri;
    auto fref = config.uri2FileRef(gs, uri);
    if (!fref.exists()) {
        response->result = std::move(emptyResult);
        return response;
    }
    auto pos = *params->position;
    auto queryLoc = config.lspPos2Loc(fref, pos, gs);
    if (!queryLoc.exists()) {
        response->result = std::move(emptyResult);
        return response;
    }
    auto result = queryByLoc(typechecker, uri, pos, LSPMethod::TextDocumentCompletion);

    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    vector<unique_ptr<CompletionItem>> items;
    if (queryResponses.empty()) {
        response->result = std::move(emptyResult);
        return response;
    }

    auto resp = move(queryResponses[0]);

    if (auto sendResp = resp->isSend()) {
        auto callerSideName = sendResp->callerSideName;
        auto prefix = (callerSideName == core::Names::methodNameMissing() || !sendResp->funLoc.contains(queryLoc))
                          ? ""
                          : callerSideName.shortName(gs);
        if (prefix == "" && queryLoc.adjust(gs, -2, 0).source(gs) == "::") {
            // Probably a case like this:
            //   A::|
            //   puts
            // which parses as a method call (A.puts()) instead of a constant.

            auto scopes = core::lsp::ConstantResponse::Scopes{};
            // We're ignoring the other dispatch components here, because the fact that we saw `::`
            // in the source means that it's likely a constant lit, not something with a fancy type.
            // Also since it's a constant lit, it probably has type `T.class_of(...)`, so we want
            // the represented type for the sake of constant lookup.
            auto recv = core::Types::getRepresentedClass(gs, sendResp->dispatchResult->main.receiver);
            if (recv.exists()) {
                scopes.emplace_back(recv);
            }

            auto params = SearchParams{
                queryLoc,
                prefix,
                nullopt,           // do not suggest methods
                false,             // do not suggest keywords
                core::MethodRef{}, // do not suggest locals
                scopes,
            };
            items = this->getCompletionItems(typechecker, params);
        } else {
            // isPrivateOk means that there is no syntactic receiver. This check prevents completing `x.de` to `x.def`
            // (If there is a method whose name overlaps with a keyword, it will still show up as a _method_ item.)
            auto suggestKeywords = sendResp->isPrivateOk;
            auto params = SearchParams{
                queryLoc, prefix,
                MethodSearchParams{
                    sendResp->dispatchResult,
                    sendResp->totalArgs,
                    sendResp->isPrivateOk,
                },
                suggestKeywords,
                // No receiver means that local variables are allowed here.
                sendResp->isPrivateOk ? sendResp->enclosingMethod : core::MethodRef{},
                core::lsp::ConstantResponse::Scopes{}, // constants don't make sense here
            };
            items = this->getCompletionItems(typechecker, params);
        }
    } else if (auto constantResp = resp->isConstant()) {
        SearchParams params;
        if (constantResp->name == core::Names::Constants::ErrorNode()) {
            // We're only getting a ConstantResult from the LSP because that's how we model "the
            // user typed nothing.
            params = searchParamsForEmptyAssign(gs, queryLoc, constantResp->enclosingMethod, constantResp->scopes);
        } else {
            // Normal constant response.
            auto name = constantResp->name;
            auto prefix = name == core::Names::Constants::ConstantNameMissing() ? "" : name.shortName(gs);
            if (prefix != "") {
                auto behindCursor = queryLoc.adjust(gs, -1 * static_cast<int32_t>(prefix.size()), 0);
                auto precedingChar = queryLoc.adjust(gs, -1, 0);
                if (behindCursor.source(gs) != prefix && precedingChar.source(gs) == ":") {
                    // Probably a case like this:
                    //   A::|
                    //   B::C
                    // which parses (so we don't get ConstantNameMissing), but we don't want to use
                    // `B` as the completion prefix.
                    prefix = "";
                }
            }
            auto methodSearchParams = nullopt;
            auto suggestKeywords = false;
            auto enclosingMethod = core::MethodRef{};
            params = SearchParams{
                queryLoc, prefix, methodSearchParams, suggestKeywords, enclosingMethod, std::move(constantResp->scopes),
            };
        }
        items = this->getCompletionItems(typechecker, params);
    } else if (auto identResp = resp->isIdent()) {
        auto varName = identResp->variable._name.shortName(gs);
        auto nameLen = static_cast<int32_t>(varName.size());

        auto termLocPrefix = identResp->termLoc.adjustLen(gs, 0, nameLen);

        if (queryLoc.adjustLen(gs, -1 * nameLen, nameLen).source(gs) == varName) {
            // Cursor at end of variable name
            auto suggestKeywords = true;
            auto prefix = varName;
            auto params = SearchParams{
                queryLoc,
                prefix,
                methodSearchParamsForEmptyAssign(gs, identResp->enclosingMethod),
                suggestKeywords,
                identResp->enclosingMethod,
                core::lsp::ConstantResponse::Scopes{identResp->enclosingMethod.data(gs)->owner},
            };
            items = this->getCompletionItems(typechecker, params);
        } else if (termLocPrefix.source(gs) == varName && !termLocPrefix.contains(queryLoc)) {
            // This is *probably* (but not definitely necessarily) an IdentResponse for an
            // assignment, with the cursor somewhere on the RHS of the `=` but before having typed
            // anything. This case is super common for code like this (cursor is `|`):
            //     x =|
            //     y = nil
            // This technically parses and comes back as an IdentResponse for the whole `x =`
            // assignment. Let's just toss that away and suggest with an empty prefix.
            auto params = searchParamsForEmptyAssign(gs, queryLoc, identResp->enclosingMethod, {});
            items = this->getCompletionItems(typechecker, params);
        }
    }

    response->result = make_unique<CompletionList>(false, move(items));
    return response;
}

} // namespace sorbet::realmain::lsp
