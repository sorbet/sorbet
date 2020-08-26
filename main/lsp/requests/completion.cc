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

vector<core::SymbolRef> ancestorsImpl(const core::GlobalState &gs, core::SymbolRef sym, vector<core::SymbolRef> &&acc) {
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
vector<core::SymbolRef> ancestors(const core::GlobalState &gs, core::SymbolRef receiver) {
    return ancestorsImpl(gs, receiver, vector<core::SymbolRef>{});
}

struct SimilarMethod final {
    int depth;
    core::SymbolRef receiver;
    core::SymbolRef method;

    // Populated later
    core::TypePtr receiverType = nullptr;
    shared_ptr<core::TypeConstraint> constr = nullptr;
};

bool hasAngleBrackets(string_view haystack) {
    return absl::c_any_of(haystack, [](char c) { return c == '<' || c == '>'; });
}

using SimilarMethodsByName = UnorderedMap<core::NameRef, vector<SimilarMethod>>;

// First of pair is "found at this depth in the ancestor hierarchy"
// Second of pair is method symbol found at that depth, with name similar to prefix.
SimilarMethodsByName similarMethodsForClass(const core::GlobalState &gs, core::SymbolRef receiver, string_view prefix) {
    auto result = SimilarMethodsByName{};

    int depth = -1;
    for (auto ancestor : ancestors(gs, receiver)) {
        depth++;
        for (auto [memberName, memberSymbol] : ancestor.data(gs)->members()) {
            if (!memberSymbol.data(gs)->isMethod()) {
                continue;
            }
            if (hasAngleBrackets(memberName.data(gs)->shortName(gs))) {
                // Gets rid of methods like `<test_foo bar>` generated by our DSL passes
                continue;
            }

            if (hasSimilarName(gs, memberName, prefix)) {
                // Creates the the list if it does not exist
                result[memberName].emplace_back(SimilarMethod{depth, receiver, memberSymbol});
            }
        }
    }

    return result;
}

// Unconditionally creates an intersection of the methods
// (for both union and intersection types, it's only valid to call a method by name if it exists on all components)
SimilarMethodsByName mergeSimilarMethods(SimilarMethodsByName left, SimilarMethodsByName right) {
    auto result = SimilarMethodsByName{};

    for (auto [methodName, leftSimilarMethods] : left) {
        if (right.find(methodName) != right.end()) {
            for (auto similarMethod : leftSimilarMethods) {
                result[methodName].emplace_back(similarMethod);
            }
            for (auto similarMethod : right[methodName]) {
                result[methodName].emplace_back(similarMethod);
            }
        }
    }
    return result;
}

SimilarMethodsByName similarMethodsForReceiver(const core::GlobalState &gs, const core::TypePtr receiver,
                                               string_view prefix) {
    auto result = SimilarMethodsByName{};

    typecase(
        receiver.get(), [&](core::ClassType *type) { result = similarMethodsForClass(gs, type->symbol, prefix); },
        [&](core::AppliedType *type) { result = similarMethodsForClass(gs, type->klass, prefix); },
        [&](core::AndType *type) {
            result = mergeSimilarMethods(similarMethodsForReceiver(gs, type->left, prefix),
                                         similarMethodsForReceiver(gs, type->right, prefix));
        },
        [&](core::ProxyType *type) { result = similarMethodsForReceiver(gs, type->underlying(), prefix); },
        [&](core::Type *type) { return; });

    return result;
}

// Walk a core::DispatchResult to find methods similar to `prefix` on any of its DispatchComponents' receivers.
SimilarMethodsByName allSimilarMethods(const core::GlobalState &gs, core::DispatchResult &dispatchResult,
                                       string_view prefix) {
    auto result = similarMethodsForReceiver(gs, dispatchResult.main.receiver, prefix);

    // Convert to shared_ptr and take ownership
    shared_ptr<core::TypeConstraint> constr = move(dispatchResult.main.constr);

    for (auto &[methodName, similarMethods] : result) {
        for (auto &similarMethod : similarMethods) {
            ENFORCE(similarMethod.receiverType == nullptr, "About to overwrite non-null receiverType");
            similarMethod.receiverType = dispatchResult.main.receiver;

            ENFORCE(similarMethod.constr == nullptr, "About to overwrite non-null constr");
            similarMethod.constr = constr;
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

    auto result = vector<RubyKeyword>{};
    for (const auto &rubyKeyword : rubyKeywords) {
        if (absl::StartsWith(rubyKeyword.keyword, prefix)) {
            result.emplace_back(rubyKeyword);
        }
    }

    // The result is trivially sorted because we walked rubyKeywords (which is sorted) in order.
    return result;
}

vector<core::LocalVariable> allSimilarLocals(const core::GlobalState &gs, const vector<core::LocalVariable> &locals,
                                             string_view prefix) {
    auto result = vector<core::LocalVariable>{};
    for (const auto &local : locals) {
        if (hasSimilarName(gs, local._name, prefix)) {
            result.emplace_back(local);
        }
    }

    return result;
}

string methodSnippet(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiverType,
                     const core::TypeConstraint *constraint) {
    fmt::memory_buffer result;
    fmt::format_to(result, "{}", method.data(gs)->name.data(gs)->shortName(gs));
    auto nextTabstop = 1;

    vector<string> typeAndArgNames;
    for (auto &argSym : method.data(gs)->arguments()) {
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
            fmt::format_to(argBuf, "{}: ", argSym.name.data(gs)->shortName(gs));
        }
        if (argSym.type) {
            auto resultType = getResultType(gs, argSym.type, method, receiverType, constraint)->show(gs);
            fmt::format_to(argBuf, "${{{}:{}}}", nextTabstop++, resultType);
        } else {
            fmt::format_to(argBuf, "${{{}}}", nextTabstop++);
        }
        typeAndArgNames.emplace_back(to_string(argBuf));
    }

    if (!typeAndArgNames.empty()) {
        fmt::format_to(result, "({})", fmt::join(typeAndArgNames, ", "));
    }

    ENFORCE(!method.data(gs)->arguments().empty());
    auto &blkArg = method.data(gs)->arguments().back();
    ENFORCE(blkArg.flags.isBlock);

    auto hasBlockType = blkArg.type != nullptr && !blkArg.type->isUntyped();
    if (hasBlockType && !core::Types::isSubType(gs, core::Types::nilClass(), blkArg.type)) {
        string blkArgs;
        if (auto *appliedType = core::cast_type<core::AppliedType>(blkArg.type.get())) {
            if (appliedType->targs.size() >= 2) {
                // The first element in targs is the return type.
                auto targs_it = appliedType->targs.begin();
                targs_it++;
                blkArgs = fmt::format(" |{}|", fmt::map_join(targs_it, appliedType->targs.end(), ", ", [&](auto targ) {
                                          auto resultType = getResultType(gs, targ, method, receiverType, constraint);
                                          return fmt::format("${{{}:{}}}", nextTabstop++, resultType->show(gs));
                                      }));
            }
        }

        fmt::format_to(result, " do{}\n  ${{{}}}\nend", blkArgs, nextTabstop++);
    }

    fmt::format_to(result, "${{0}}");
    return to_string(result);
}

// This is an approximation. It takes advantage of the fact that nearly all of the time,
// the prefix being used to suggest completion items actually accurred in the source text
// of the file, immediately before the queryLoc.
//
// This is somewhat brittle, but has worked well so far.
unique_ptr<Range> replacementRangeForQuery(const core::GlobalState &gs, core::Loc queryLoc, string_view prefix) {
    auto queryStart = queryLoc.beginPos();
    u4 prefixSize = prefix.size();
    auto replacementLoc = core::Loc{queryLoc.file(), queryStart - prefixSize, queryStart};
    // Sometimes Range::fromLoc returns nullptr (commonly when running under a fuzzer which disables certain loc info).
    return Range::fromLoc(gs, replacementLoc);
}

unique_ptr<CompletionItem> getCompletionItemForKeyword(const core::GlobalState &gs, const LSPConfiguration &config,
                                                       const RubyKeyword &rubyKeyword, const core::Loc queryLoc,
                                                       string_view prefix, size_t sortIdx) {
    auto supportSnippets = config.getClientConfig().clientCompletionItemSnippetSupport;
    auto markupKind = config.getClientConfig().clientCompletionItemMarkupKind;
    auto item = make_unique<CompletionItem>(rubyKeyword.keyword);
    item->sortText = fmt::format("{:06d}", sortIdx);

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

    auto label = string(maybeAlias.data(gs)->name.data(gs)->shortName(gs));

    // Intuition for when to use maybeAlias vs what: if it needs to know the original name: maybeAlias.
    // If it needs to know the types / arity: what. Default to `what` if you don't know.
    auto what = maybeAlias.data(gs)->dealias(gs);

    auto item = make_unique<CompletionItem>(label);

    // Completion items are sorted by sortText if present, or label if not. We unconditionally use an index to sort.
    // If we ever have 100,000+ items in the completion list, we'll need to bump the padding here.
    item->sortText = fmt::format("{:06d}", sortIdx);

    if (what.data(gs)->isClassOrModule()) {
        if (what.data(gs)->isClassOrModuleClass()) {
            if (what.data(gs)->derivesFrom(gs, core::Symbols::T_Enum())) {
                item->kind = CompletionItemKind::Enum;
            } else {
                item->kind = CompletionItemKind::Class;
            }
        } else {
            if (what.data(gs)->isClassOrModuleAbstract() || what.data(gs)->isClassOrModuleInterface()) {
                item->kind = CompletionItemKind::Interface;
            } else {
                item->kind = CompletionItemKind::Module;
            }
        }
    } else if (what.data(gs)->isTypeMember() || what.data(gs)->isStaticField()) {
        item->kind = CompletionItemKind::Field;
    } else {
        ENFORCE(false, "Unhandled kind of constant in getCompletionItemForConstant");
    }

    item->detail = maybeAlias.data(gs)->show(gs);

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
    auto whatFile = what.data(gs)->loc().file();
    if (whatFile.exists()) {
        documentation = findDocumentation(whatFile.data(gs).source(), what.data(gs)->loc().beginPos());
    }

    auto prettyType = prettyTypeForConstant(gs, what);
    item->documentation = formatRubyMarkup(markupKind, prettyType, documentation);

    return item;
}

unique_ptr<CompletionItem> getCompletionItemForLocal(const core::GlobalState &gs, const LSPConfiguration &config,
                                                     const core::LocalVariable &local, const core::Loc queryLoc,
                                                     string_view prefix, size_t sortIdx) {
    auto label = string(local._name.data(gs)->shortName(gs));
    auto item = make_unique<CompletionItem>(label);
    item->sortText = fmt::format("{:06d}", sortIdx);
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

vector<core::LocalVariable> localsForMethod(const core::GlobalState &gs, LSPTypecheckerDelegate &typechecker,
                                            const core::SymbolRef method) {
    auto files = vector<core::FileRef>{};
    for (auto loc : method.data(gs)->locs()) {
        files.emplace_back(loc.file());
    }
    auto resolved = typechecker.getResolved(files);

    // Instantiate localVarFinder outside loop so that result accumualates over every time we TreeMap::apply
    LocalVarFinder localVarFinder(method);
    for (auto &t : resolved) {
        auto ctx = core::Context(gs, core::Symbols::root(), t.file);
        t.tree = ast::TreeMap::apply(ctx, localVarFinder, move(t.tree));
    }

    auto result = localVarFinder.result();
    fast_sort(result, [&gs](const auto &left, const auto &right) {
        // Sort by actual name, not by NameRef id
        if (left._name != right._name) {
            return left._name.data(gs)->shortName(gs) < right._name.data(gs)->shortName(gs);
        } else {
            return left < right;
        }
    });

    // Dedup
    auto it = unique(result.begin(), result.end());
    result.erase(it, result.end());
    return result;
}

core::SymbolRef firstMethodAfterQuery(LSPTypecheckerDelegate &typechecker, const core::Loc queryLoc) {
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

    core::SymbolRef receiverSym;
    if (auto classType = core::cast_type<core::ClassType>(receiverType.get())) {
        receiverSym = classType->symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(receiverType.get())) {
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
    item->sortText = fmt::format("{:06d}", sortIdx);
    item->detail = fmt::format("Suggested sig for {}", targetMethod.data(gs)->name.data(gs)->shortName(gs));

    u4 queryStart = queryLoc.beginPos();
    u4 prefixSize = prefix.size();
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

    if (!(sym.data(gs)->isClassOrModule() || sym.data(gs)->isStaticField() || sym.data(gs)->isTypeMember())) {
        return false;
    }

    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind != core::NameKind::CONSTANT) {
        return false;
    }

    if (name.data(gs)->isTEnumName(gs)) {
        // Every T::Enum value gets a class with the ~same name (see rewriter/TEnum.cc for details).
        // This manifests as showing two completion results when we should only show one, so skip the bad kind.
        return false;
    }

    if (hasAngleBrackets(name.data(gs)->shortName(gs))) {
        // Gets rid of classes like `<Magic>`; they can't be typed by a user anyways.
        return false;
    }

    return hasSimilarName(gs, name, prefix);
}

} // namespace

CompletionTask::CompletionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CompletionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCompletion), params(move(params)) {}

unique_ptr<CompletionItem>
CompletionTask::getCompletionItemForMethod(LSPTypecheckerDelegate &typechecker, core::SymbolRef maybeAlias,
                                           core::TypePtr receiverType, const core::TypeConstraint *constraint,
                                           core::Loc queryLoc, string_view prefix, size_t sortIdx) const {
    const auto &gs = typechecker.state();
    ENFORCE(maybeAlias.exists());
    ENFORCE(maybeAlias.data(gs)->isMethod());
    auto clientConfig = config.getClientConfig();
    auto supportsSnippets = clientConfig.clientCompletionItemSnippetSupport;
    auto markupKind = clientConfig.clientCompletionItemMarkupKind;

    auto label = string(maybeAlias.data(gs)->name.data(gs)->shortName(gs));

    // Intuition for when to use maybeAlias vs what: if it needs to know the original name: maybeAlias.
    // If it needs to know the types / arity: what. Default to `what` if you don't know.
    auto what = maybeAlias.data(gs)->dealias(gs);

    if (what == core::Symbols::sig()) {
        if (auto item = trySuggestSig(typechecker, clientConfig, what, receiverType, queryLoc, prefix, sortIdx)) {
            return item;
        }
    }

    auto item = make_unique<CompletionItem>(label);

    // Completion items are sorted by sortText if present, or label if not. We unconditionally use an index to sort.
    // If we ever have 100,000+ items in the completion list, we'll need to bump the padding here.
    item->sortText = fmt::format("{:06d}", sortIdx);

    item->kind = CompletionItemKind::Method;
    item->detail = maybeAlias.data(gs)->show(gs);

    string replacementText;
    if (supportsSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
        replacementText = methodSnippet(gs, what, receiverType, constraint);
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

void CompletionTask::findSimilarConstants(const core::GlobalState &gs, const core::lsp::ConstantResponse &resp,
                                          core::Loc queryLoc, vector<unique_ptr<CompletionItem>> &items) const {
    auto prefix = resp.name.data(gs)->shortName(gs);
    config.logger->debug("Looking for constant similar to {}", prefix);
    ENFORCE(!resp.scopes.empty());

    if (resp.scopes.size() == 1 && !resp.scopes[0].exists()) {
        // This happens when there was a contant literal like C::D but `C` itself was stubbed,
        // so we have no idea what `D` is or what its resolution scope is.
        return;
    }

    for (auto scope : resp.scopes) {
        // TODO(jez) This membersStableOrderSlow is the only ordering we have on constant items right now.
        // We should probably at least sort by whether the prefix of the suggested constant matches.
        for (auto [_name, sym] : scope.data(gs)->membersStableOrderSlow(gs)) {
            if (isSimilarConstant(gs, prefix, sym)) {
                items.push_back(getCompletionItemForConstant(gs, config, sym, queryLoc, prefix, items.size()));
            }
        }
    }

    if (resp.scopes.size() == 1) {
        // If scope is size one, that means we were either given an explicit scope (::A, B::C),
        // or we've been requested to resolve a bare constant at the top level.
        // In either case, we want to skip looking through ancestors and instead suggest constants only on that scope.
        return;
    }

    int i = -1;
    for (auto ancestor : ancestors(gs, resp.scopes[0])) {
        i++;

        if (i == 0) {
            // Skip first ancestor; it already showed up in the search over nesting scope.
            continue;
        }

        for (auto [_name, sym] : ancestor.data(gs)->membersStableOrderSlow(gs)) {
            if (isSimilarConstant(gs, prefix, sym)) {
                items.push_back(getCompletionItemForConstant(gs, config, sym, queryLoc, prefix, items.size()));
            }
        }
    }
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
        auto prefix = sendResp->callerSideName.data(gs)->shortName(gs);
        config.logger->debug("Looking for method similar to {}", prefix);

        // isPrivateOk means that there is no syntactic receiver. This check prevents completing `x.de` to `x.def`
        auto similarKeywords = sendResp->isPrivateOk ? allSimilarKeywords(prefix) : vector<RubyKeyword>{};

        auto similarMethodsByName = allSimilarMethods(gs, *sendResp->dispatchResult, prefix);
        for (auto &[methodName, similarMethods] : similarMethodsByName) {
            fast_sort(similarMethods, [&](const auto &left, const auto &right) -> bool {
                if (left.depth != right.depth) {
                    return left.depth < right.depth;
                }

                return left.method._id < right.method._id;
            });
        }

        auto locals = localsForMethod(gs, typechecker, sendResp->enclosingMethod);

        auto similarLocals =
            sendResp->isPrivateOk ? allSimilarLocals(gs, locals, prefix) : vector<core::LocalVariable>{};

        auto deduped = vector<SimilarMethod>{};
        for (auto &[methodName, similarMethods] : similarMethodsByName) {
            if (methodName.data(gs)->kind == core::NameKind::UNIQUE &&
                methodName.data(gs)->unique.uniqueNameKind == core::UniqueNameKind::MangleRename) {
                // It's possible we want to ignore more things here. But note that we *don't* want to ignore all
                // unique names, because we want each overload to show up but those use unique names.
                continue;
            }

            // Since each list is sorted by depth, taking the first elem dedups by depth within each name.
            auto similarMethod = similarMethods[0];

            if (similarMethod.method.data(gs)->isPrivate() && !sendResp->isPrivateOk) {
                continue;
            }

            deduped.emplace_back(similarMethod);
        }

        fast_sort(deduped, [&](const auto &left, const auto &right) -> bool {
            if (left.depth != right.depth) {
                return left.depth < right.depth;
            }

            auto leftShortName = left.method.data(gs)->name.data(gs)->shortName(gs);
            auto rightShortName = right.method.data(gs)->name.data(gs)->shortName(gs);
            if (leftShortName != rightShortName) {
                if (absl::StartsWith(leftShortName, prefix) && !absl::StartsWith(rightShortName, prefix)) {
                    return true;
                }
                if (!absl::StartsWith(leftShortName, prefix) && absl::StartsWith(rightShortName, prefix)) {
                    return false;
                }

                return leftShortName < rightShortName;
            }

            return left.method._id < right.method._id;
        });

        // TODO(jez) Do something smarter here than "all keywords then all locals then all methods"
        for (auto &similarKeyword : similarKeywords) {
            items.push_back(getCompletionItemForKeyword(gs, config, similarKeyword, queryLoc, prefix, items.size()));
        }
        for (auto &similarLocal : similarLocals) {
            items.push_back(getCompletionItemForLocal(gs, config, similarLocal, queryLoc, prefix, items.size()));
        }
        for (auto &similarMethod : deduped) {
            items.push_back(getCompletionItemForMethod(typechecker, similarMethod.method, similarMethod.receiverType,
                                                       similarMethod.constr.get(), queryLoc, prefix, items.size()));
        }
    } else if (auto constantResp = resp->isConstant()) {
        findSimilarConstants(gs, *constantResp, queryLoc, items);
    }

    response->result = make_unique<CompletionList>(false, move(items));
    return response;
}

} // namespace sorbet::realmain::lsp
