#include "main/lsp/requests/completion.h"
#include "absl/algorithm/container.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/typecase.h"
#include "core/lsp/QueryResponse.h"
#include "core/source_generator/source_generator.h"
#include "main/lsp/FieldFinder.h"
#include "main/lsp/KwargsFinder.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/LocalVarFinder.h"
#include "main/lsp/NextMethodFinder.h"
#include "main/lsp/json_types.h"

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

using KeywordLikeSnippet = RubyKeyword;

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

// Since these are not technically Ruby keywords but will be treated as such we store
// these separately for hygiene.
const KeywordLikeSnippet keywordLikeSnippets[] = {
    {"enum", "Creates an enum class", "class ${1:EnumName} < T::Enum\n  enums do\n    $0\n  end\nend"},
    {"struct", "Creates a new struct class", "class ${1:StructName} < T::Struct\n  $0\nend"},
};

vector<core::ClassOrModuleRef> ancestorsImpl(const core::GlobalState &gs, core::ClassOrModuleRef sym,
                                             vector<core::ClassOrModuleRef> &&acc) {
    // The implementation here is similar to Symbols::derivesFrom.
    ENFORCE(sym.data(gs)->flags.isLinearizationComputed);
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

bool hasAngleBrackets(string_view haystack) {
    return absl::c_any_of(haystack, [](char c) { return c == '<' || c == '>'; });
}

// Note: this will always return `true` for `pattern == ""`.
bool hasSimilarName(const core::GlobalState &gs, core::NameRef name, string_view pattern) {
    string_view view = name.shortName(gs);
    auto fnd = view.find(pattern);
    return fnd != string_view::npos;
}

bool hasPrefixedName(const core::GlobalState &gs, core::NameRef name, string_view pattern) {
    string_view view = name.shortName(gs);
    return absl::StartsWith(view, pattern);
}

using SimilarMethod = CompletionTask::SimilarMethod;
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
            if (memberName.isOverloadName(gs)) {
                // We skip overloads here, relying on the original method def to show up in the results. As that will be
                // flagged as having overloads, we modify the help output to include all overload sigs.
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
SimilarMethodsByName mergeSimilarMethods(SimilarMethodsByName &&left, SimilarMethodsByName &&right) {
    auto result = SimilarMethodsByName{};

    for (auto &[methodName, leftSimilarMethods] : left) {
        auto it = right.find(methodName);
        if (it == right.end()) {
            continue;
        }

        auto &methods = result[methodName];

        methods.insert(methods.end(), make_move_iterator(leftSimilarMethods.begin()),
                       make_move_iterator(leftSimilarMethods.end()));
        methods.insert(methods.end(), make_move_iterator(it->second.begin()), make_move_iterator(it->second.end()));
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
        [&](const core::LambdaParam &type) { result = similarMethodsForReceiver(gs, type.upperBound, prefix); },
        [&](const core::SelfTypeParam &type) {
            result = similarMethodsForReceiver(gs, type.definition.resultType(gs), prefix);
        },
        [&](const core::TypePtr &type) {
            if (is_proxy_type(receiver)) {
                result = similarMethodsForReceiver(gs, receiver.underlying(gs), prefix);
            }
        });

    return result;
}

// Walk a core::DispatchResult to find methods similar to `prefix` on any of its DispatchComponents' receivers.
SimilarMethodsByName allSimilarMethods(const core::GlobalState &gs, const core::DispatchResult &dispatchResult,
                                       string_view prefix) {
    ENFORCE(!dispatchResult.main.receiver.isUntyped())

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
        result = mergeSimilarMethods(std::move(result), allSimilarMethods(gs, *dispatchResult.secondary, prefix));
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

vector<KeywordLikeSnippet> allSimilarLikeKeywords(string_view prefix) {
    ENFORCE(
        absl::c_is_sorted(keywordLikeSnippets, [](auto &left, auto &right) { return left.keyword < right.keyword; }),
        "keywordLikeSnippets is not sorted by keyword; completion results will be out of order");

    if (prefix == "") {
        // Since we suggest keyword snippets first, they're just noise when the prefix is empty
        return {};
    }

    auto result = vector<KeywordLikeSnippet>{};
    for (const auto &keywordSnippet : keywordLikeSnippets) {
        if (absl::StartsWith(keywordSnippet.keyword, prefix)) {
            result.emplace_back(keywordSnippet);
        }
    }

    // The result is trivially sorted because we walked keywordLikeSnippets (which is sorted) in order.
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

string methodSnippet(const core::GlobalState &gs, core::DispatchResult &dispatchResult, core::MethodRef maybeAlias,
                     const core::TypePtr &receiverType, uint16_t totalArgs, core::Loc queryLoc) {
    fmt::memory_buffer result;
    auto shortName = maybeAlias.data(gs)->name.shortName(gs);
    auto isSetter = maybeAlias.data(gs)->name.isSetter(gs);
    if (isSetter) {
        fmt::format_to(std::back_inserter(result), "{}", string_view(shortName.data(), shortName.size() - 1));
    } else {
        fmt::format_to(std::back_inserter(result), "{}", shortName);
    }

    /* If we are completing an existing send that either has some arguments
     * or a block specified already then simply complete the method name
     * since the rest is likely useless
     */
    if (totalArgs > 0 || dispatchResult.main.blockReturnType != nullptr) {
        auto maybeNewline = queryLoc.adjustLen(gs, 0, 1);
        // ... but carve out an exception if the query location is at the end of the line, because
        // then likely it only looks like there are "arguments" to this method call because of how
        // Ruby allows the `x.` being split from the method name on the next line
        //
        // (This isn't a great solution, but I think that we're likely going to revisit generating
        // snippets here in the future with a bit of an overhaul of how completion works, so I'm
        // fine with it in the mean time.)
        if (!maybeNewline.exists() || maybeNewline.source(gs) != "\n") {
            fmt::format_to(std::back_inserter(result), "${{0}}");
            return to_string(result);
        }
    }

    if (isSetter) {
        fmt::format_to(std::back_inserter(result), " = ${{0}}");
        return to_string(result);
    }

    auto method = maybeAlias.data(gs)->dealiasMethod(gs);
    auto &parameters = method.data(gs)->parameters;
    auto requiresArguments = absl::c_any_of(parameters, [](const auto &param) {
        return !param.flags.isDefault && !param.flags.isRepeated && !param.flags.isBlock;
    });
    if (requiresArguments) {
        fmt::format_to(back_inserter(result), "(${{0}})");
        return to_string(result);
    }

    ENFORCE(!method.data(gs)->parameters.empty());
    if (parameters.size() == 1) {
        auto &blkParam = method.data(gs)->parameters.back();
        ENFORCE(blkParam.flags.isBlock);

        auto hasBlockType = blkParam.type != nullptr && !blkParam.type.isUntyped() && !blkParam.type.isBottom() &&
                            blkParam.type != core::Types::nilClass();
        if (hasBlockType && !core::Types::isSubType(gs, core::Types::nilClass(), blkParam.type)) {
            // If the method has no parameters except a required block, put the cursor inside `{|}`.
            fmt::format_to(back_inserter(result), " {{${{0}}}}");
            return to_string(result);
        }
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

    item->insertText = replacementText;

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
        if (whatKlass.data(gs)->isClass()) {
            if (whatKlass.data(gs)->derivesFrom(gs, core::Symbols::T_Enum())) {
                item->kind = CompletionItemKind::Enum;
            } else {
                item->kind = CompletionItemKind::Class;
            }
        } else {
            if (whatKlass.data(gs)->flags.isAbstract || whatKlass.data(gs)->flags.isInterface) {
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

unique_ptr<CompletionItem> getCompletionItemForKwarg(const core::GlobalState &gs, const LSPConfiguration &config,
                                                     const core::NameRef local, const core::Loc queryLoc,
                                                     string_view prefix, size_t sortIdx) {
    auto label = local.shortName(gs);
    auto item = make_unique<CompletionItem>(absl::StrCat(label, ": (keyword argument)"));
    item->sortText = formatSortIndex(sortIdx);
    item->kind = CompletionItemKind::Property;

    auto replacementText = absl::StrCat(label, ":");
    if (auto replacementRange = replacementRangeForQuery(gs, queryLoc, prefix)) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), move(replacementText));
    } else {
        item->insertText = move(replacementText);
    }
    item->insertTextFormat = InsertTextFormat::PlainText;

    return item;
}

unique_ptr<CompletionItem> getCompletionItemForLocalName(const core::GlobalState &gs, const LSPConfiguration &config,
                                                         const core::NameRef local, const core::Loc queryLoc,
                                                         string_view prefix, size_t sortIdx) {
    auto label = string(local.shortName(gs));
    auto item = make_unique<CompletionItem>(label);
    item->sortText = formatSortIndex(sortIdx);
    item->kind = absl::StartsWith(prefix, "@") ? CompletionItemKind::Field : CompletionItemKind::Variable;

    auto replacementText = label;
    if (auto replacementRange = replacementRangeForQuery(gs, queryLoc, prefix)) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
    } else {
        item->insertText = replacementText;
    }
    item->insertTextFormat = InsertTextFormat::PlainText;

    return item;
}

vector<core::NameRef> allSimilarFields(const core::GlobalState &gs, core::ClassOrModuleRef klass, string_view prefix) {
    vector<core::NameRef> result;

    // `ancestors` already includes klass, so we don't have to handle klass specially
    // as we do in allSimilarConstantItems.
    for (auto ancestor : ancestors(gs, klass)) {
        for (auto [name, sym] : ancestor.data(gs)->members()) {
            if (!sym.isFieldOrStaticField()) {
                continue;
            }

            // TODO: this does prefix matching for instance/class variables, but our
            // completion for locals matches anywhere in the name
            if (hasPrefixedName(gs, name, prefix)) {
                result.emplace_back(name);
            }
        }
    }

    fast_sort(result, [&gs](const auto &left, const auto &right) {
        // Sort by actual name, not by NameRef id
        if (left != right) {
            return left.shortName(gs) < right.shortName(gs);
        } else {
            return left.rawId() < right.rawId();
        }
    });

    auto it = unique(result.begin(), result.end());
    result.erase(it, result.end());

    return result;
}

vector<core::NameRef> allSimilarFieldsForClass(LSPTypecheckerDelegate &typechecker, const core::ClassOrModuleRef klass,
                                               const core::Loc queryLoc, const ast::ParsedFile &resolved,
                                               ast::UnresolvedIdent::Kind kind, string_view prefix) {
    const auto &gs = typechecker.state();

    // We have an interesting problem here: the symbol table already stores
    // information about all the fields in a class, but we only populate the
    // symbol table with this information when the fields are typed in some
    // way.  But we would like to provide completion for all fields, typed
    // or not.
    //
    // The compromise we take is this: if the file is typed < StrictLevel::Strict,
    // we walk the AST to discover the fields in that class and we add those
    // results to the symbol table results.  We might discover duplicate information
    // (people might have declared instance variables as typed in StrictLevel::True
    // files, but that's OK, since we can't know apriori what fields we would get
    // from which source.
    auto result = allSimilarFields(gs, klass, prefix);

    if (resolved.file.data(gs).strictLevel < core::StrictLevel::Strict) {
        vector<core::NameRef> fields;
        FieldFinder fieldFinder(klass, kind, fields);
        auto ctx = core::Context(gs, core::Symbols::root(), resolved.file);
        ast::ConstTreeWalk::apply(ctx, fieldFinder, resolved.tree);

        // TODO: this does prefix matching for instance/class variables, but our
        // completion for locals matches anywhere in the name
        auto it = remove_if(fields.begin(), fields.end(),
                            [&gs, &prefix](auto name) { return !hasPrefixedName(gs, name, prefix); });
        result.insert(result.end(), fields.begin(), it);
    }

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

vector<core::NameRef> localNamesForMethod(LSPTypecheckerDelegate &typechecker, const core::MethodRef method,
                                          const core::Loc queryLoc, const ast::ParsedFile &resolved) {
    const auto &gs = typechecker.state();

    vector<core::NameRef> result;
    LocalVarFinder localVarFinder(method, queryLoc, result);
    auto ctx = core::Context(gs, core::Symbols::root(), resolved.file);
    ast::ConstTreeWalk::apply(ctx, localVarFinder, resolved.tree);

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

core::MethodRef firstMethodAfterQuery(LSPTypecheckerDelegate &typechecker, const core::Loc queryLoc,
                                      const ast::ParsedFile &resolved) {
    const auto &gs = typechecker.state();

    NextMethodFinder nextMethodFinder(queryLoc);
    auto ctx = core::Context(gs, core::Symbols::root(), resolved.file);
    ast::ConstTreeWalk::apply(ctx, nextMethodFinder, resolved.tree);

    return nextMethodFinder.result();
}

// This code is ugly but I'm convinced it's because of C++'s baroque string APIs, not for lack
// of trying to make this code prettier. If you're up to the challenge, feel free.
string suggestedSigToSnippet(string_view suggestedSig) {
    auto result = fmt::format("{}${{0}}", suggestedSig);
    constexpr string_view needle = "T.untyped"sv;

    auto tabstopId = 1;
    size_t replaceFrom = 0;
    while (true) {
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
                                         core::TypePtr receiverType, const core::Loc queryLoc,
                                         const ast::ParsedFile &resolved, string_view prefix, size_t sortIdx) {
    ENFORCE(receiverType != nullptr);

    // Completion with T::Sig::WithoutRuntime.sig / Sorbet::Private::Static.sig won't work because
    // this code path relies on the DispatchResult's receiver type being the `self` of the module
    // where the completion is happening, which isn't true for those two.  Luckily, this won't
    // happen in practice, because SigSuggestion.cc short circuits if the method already has a sig.
    ENFORCE(what == core::Symbols::sig());

    const auto &gs = typechecker.state();
    const auto markupKind = clientConfig.clientCompletionItemMarkupKind;
    const auto supportSnippets = clientConfig.clientCompletionItemSnippetSupport;

    auto targetMethod = firstMethodAfterQuery(typechecker, queryLoc, resolved);
    if (!targetMethod.exists()) {
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

unique_ptr<CompletionItem> trySuggestYardSnippet(LSPTypecheckerDelegate &typechecker,
                                                 const LSPClientConfiguration &clientConfig, core::Loc queryLoc,
                                                 const ast::ParsedFile &resolved) {
    const auto &gs = typechecker.state();

    auto method = firstMethodAfterQuery(typechecker, queryLoc, resolved);
    if (!method.exists()) {
        return nullptr;
    }

    auto item = make_unique<CompletionItem>("##");
    item->kind = CompletionItemKind::Snippet;
    item->detail = fmt::format("YARD doc snippet for {}", method.data(gs)->name.shortName(gs));
    auto insertRange = Range::fromLoc(gs, queryLoc);

    const auto supportSnippets = clientConfig.clientCompletionItemSnippetSupport;
    if (supportSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
    } else {
        item->insertTextFormat = InsertTextFormat::PlainText;
    }

    string yardSnippetText = "\n";

    if (supportSnippets) {
        yardSnippetText += "# ${1:Summary}";
    } else {
        yardSnippetText += "# Summary";
    }
    bool firstAfterSummary = true;

    const auto &parameters = method.data(gs)->parameters;
    auto resultType = method.data(gs)->resultType;

    // 0 is final tabstop. 1 is initial tabstop (for summary)
    auto tabStop = 1;
    for (const auto &param : parameters) {
        auto argumentName = param.parameterName(gs);
        if (hasAngleBrackets(argumentName)) {
            continue;
        }

        tabStop++;

        if (firstAfterSummary) {
            firstAfterSummary = false;
            yardSnippetText += "\n#";
        }

        // TODO(jez) Might be nice to use @yieldparam / @yieldreturn for the block arg.
        if (supportSnippets) {
            yardSnippetText += fmt::format("\n# @param {} ${{{}:TODO}}", argumentName, tabStop);
        } else {
            yardSnippetText += fmt::format("\n# @param {} TODO", argumentName);
        }
    }

    if (resultType != core::Types::void_()) {
        if (firstAfterSummary) {
            firstAfterSummary = false;
            yardSnippetText += "\n#";
        }

        tabStop++;

        if (supportSnippets) {
            yardSnippetText += fmt::format("\n# @return ${{{}:TODO}}", tabStop);
        } else {
            yardSnippetText += fmt::format("\n# @return TODO");
        }
    }

    if (insertRange != nullptr) {
        item->textEdit = make_unique<TextEdit>(std::move(insertRange), string(yardSnippetText));
    } else {
        item->insertText = yardSnippetText;
    }

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
        // This happens when there was a constant literal like C::D but `C` itself was stubbed,
        // so we have no idea what `D` is or what its resolution scope is.
        return items;
    }

    for (auto scope : scopes) {
        if (!scope.isClassOrModule()) {
            continue;
        }

        for (auto [_name, sym] : scope.asClassOrModuleRef().data(gs)->membersStableOrderSlowPredicate(
                 gs, [&gs, prefix](const auto _name, const auto sym) -> bool {
                     return isSimilarConstant(gs, prefix, sym);
                 })) {
            items.push_back(
                getCompletionItemForConstant(gs, config, sym, queryLoc, prefix, initialSortIdx + items.size()));
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

        for (auto [_name, sym] : ancestor.data(gs)->membersStableOrderSlowPredicate(
                 gs, [&gs, prefix](const auto _name, const auto sym) -> bool {
                     return isSimilarConstant(gs, prefix, sym);
                 })) {
            items.push_back(
                getCompletionItemForConstant(gs, config, sym, queryLoc, prefix, initialSortIdx + items.size()));
        }
    }

    return items;
}

struct MethodResults {
    vector<SimilarMethod> methods;
    vector<SimilarMethod> operators;
};

const string OPERATOR_CHARS = "+-*/%&|^><=!~[]`:.";

MethodResults computeDedupedMethods(const core::GlobalState &gs, SimilarMethodsByName &similarMethodsByName,
                                    bool isPrivateOk, string_view prefix) {
    MethodResults result;

    Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".determine_methods");
    for (auto &[methodName, similarMethods] : similarMethodsByName) {
        fast_sort(similarMethods, [&](const auto &left, const auto &right) -> bool {
            if (left.depth != right.depth) {
                return left.depth < right.depth;
            }

            return left.method.id() < right.method.id();
        });
    }

    for (auto &[methodName, similarMethods] : similarMethodsByName) {
        if (methodName.hasUniqueNameKind(gs, core::UniqueNameKind::MangleRename)) {
            // It's possible we want to ignore more things here. But note that we *don't* want to ignore all
            // unique names, because we want each overload to show up but those use unique names.
            continue;
        }

        // Since each list is sorted by depth, taking the first elem dedups by depth within each name.
        auto similarMethod = similarMethods[0];

        if (similarMethod.method.data(gs)->flags.isPrivate && !isPrivateOk) {
            continue;
        }

        auto name = methodName.shortName(gs);
        ENFORCE(!name.empty());
        if (absl::c_contains(OPERATOR_CHARS, name.front())) {
            result.operators.emplace_back(similarMethod);
        } else {
            result.methods.emplace_back(similarMethod);
        }
    }

    auto compareMethods = [&](const auto &left, const auto &right) -> bool {
        if (left.depth != right.depth) {
            return left.depth < right.depth;
        }

        auto leftShortName = left.method.data(gs)->name.shortName(gs);
        auto rightShortName = right.method.data(gs)->name.shortName(gs);
        if (leftShortName != rightShortName) {
            if (absl::StartsWith(leftShortName, prefix) && !absl::StartsWith(rightShortName, prefix)) {
                return true;
            }
            if (!absl::StartsWith(leftShortName, prefix) && absl::StartsWith(rightShortName, prefix)) {
                return false;
            }

            return leftShortName < rightShortName;
        }

        return left.method.id() < right.method.id();
    };

    fast_sort(result.methods, compareMethods);
    fast_sort(result.operators, compareMethods);

    return result;
}

struct EnclosingSend {
    core::LocOffsets funLoc;
    core::MethodRef method;
};

EnclosingSend findEnclosingSend(const core::GlobalState &gs,
                                absl::Span<unique_ptr<core::lsp::QueryResponse>> responses) {
    EnclosingSend result;

    for (auto &resp : responses) {
        // Walking past a method definition means we're out of the context of a send.
        if (resp->isMethodDef()) {
            break;
        }

        if (auto *enclosingSend = resp->isSend()) {
            result.funLoc = enclosingSend->funLoc().offsets();
            result.method = enclosingSend->dispatchResult->main.method;
            break;
        }
    }

    return result;
}

} // namespace

CompletionTask::CompletionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CompletionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCompletion), params(move(params)) {}

unique_ptr<CompletionItem> CompletionTask::getCompletionItemForUntyped(const core::GlobalState &gs, core::Loc queryLoc,
                                                                       size_t sortIdx, string_view message) {
    string label(message);
    auto item = make_unique<CompletionItem>(label);
    item->sortText = formatSortIndex(sortIdx);
    item->kind = CompletionItemKind::Method;
    item->insertTextFormat = InsertTextFormat::PlainText;
    item->textEdit = make_unique<TextEdit>(Range::fromLoc(gs, queryLoc.copyWithZeroLength()), "");
    return item;
}

unique_ptr<CompletionItem> CompletionTask::getCompletionItemForMethod(
    LSPTypecheckerDelegate &typechecker, const SearchParams &params, const SimilarMethod &similarMethod,
    core::Loc queryLoc, const ast::ParsedFile &resolved, string_view prefix, size_t sortIdx, uint16_t totalArgs) const {
    const auto &gs = typechecker.state();
    auto maybeAlias = similarMethod.method;
    auto &receiverType = similarMethod.receiverType;
    ENFORCE(maybeAlias.exists());
    auto clientConfig = config.getClientConfig();
    auto markupKind = clientConfig.clientCompletionItemMarkupKind;

    auto label = string(maybeAlias.data(gs)->name.shortName(gs));

    // Intuition for when to use maybeAlias vs what: if it needs to know the original name: maybeAlias.
    // If it needs to know the types / arity: what. Default to `what` if you don't know.
    auto what = maybeAlias.data(gs)->dealiasMethod(gs);

    if (what == core::Symbols::sig()) {
        if (auto item =
                trySuggestSig(typechecker, clientConfig, what, receiverType, queryLoc, resolved, prefix, sortIdx)) {
            return item;
        }
    }

    auto isOverloaded = what.data(gs)->flags.isOverloaded;

    // Snippets are disabled for overloads, as it's not clear which sig the user wants to choose.
    auto supportsSnippets = !isOverloaded && clientConfig.clientCompletionItemSnippetSupport;

    auto item = make_unique<CompletionItem>(label);

    item->sortText = formatSortIndex(sortIdx);

    item->kind = CompletionItemKind::Method;
    item->detail = maybeAlias.show(gs);

    auto &dispatchResult = *params.forMethods->dispatchResult;

    string replacementText;
    if (supportsSnippets) {
        item->insertTextFormat = InsertTextFormat::Snippet;
        replacementText = methodSnippet(gs, dispatchResult, maybeAlias, receiverType, totalArgs, queryLoc);
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

    std::string prettyType;

    if (isOverloaded) {
        vector<string> defParts;
        auto origName = what.data(gs)->name;
        int i = 0;
        while (true) {
            ++i;
            auto overloadName = gs.lookupNameUnique(core::UniqueNameKind::Overload, origName, i);
            ENFORCE(overloadName.exists());
            auto overload = what.data(gs)->owner.data(gs)->findMethod(gs, overloadName);
            ENFORCE(overload.exists());
            defParts.emplace_back(
                core::source_generator::prettySigForMethod(gs, overload, receiverType, core::ShowOptions()));

            // The last overload signature always lacks the `isOverloaded` flag to terminate the chain.
            if (!overload.data(gs)->flags.isOverloaded) {
                break;
            }
        }

        defParts.emplace_back(core::source_generator::prettyDefForMethod(gs, what, core::ShowOptions()));

        prettyType = absl::StrJoin(defParts, "\n");

    } else {
        prettyType = core::source_generator::prettyTypeForMethod(gs, maybeAlias, receiverType,
                                                                 core::ShowOptions().withUseValidSyntax());
    }
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
        enclosingMethod,    // locals
        core::MethodRef{},  // do not suggest kwargs
        core::LocOffsets{}, // no fun loc available
        core::LocOffsets{}, // no receiver loc available
        core::lsp::ConstantResponse::Scopes{enclosingMethod.data(gs)->owner},
    };
}

vector<unique_ptr<CompletionItem>> CompletionTask::getCompletionItems(LSPTypecheckerDelegate &typechecker,
                                                                      SearchParams &params,
                                                                      const ast::ParsedFile &resolved) {
    const auto &gs = typechecker.state();

    // ----- locals -----

    vector<core::NameRef> similarLocals;
    if (params.enclosingMethod.exists()) {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".determine_locals");

        // Slyly reuse `UnresolvedIdent::Kind` to both determine what kind of
        // thing we're going to find and to determine the search space for
        // `fieldsForClass`.
        //
        // TODO: for empty prefixes, we would like to complete instance/class
        // variables along with locals.
        //
        // TODO: for a prefix of just "@", we should provide class variables
        // along with instance variables.
        auto kind = ast::UnresolvedIdent::Kind::Local;
        if (absl::StartsWith(params.prefix, "@@")) {
            kind = ast::UnresolvedIdent::Kind::Class;
        } else if (absl::StartsWith(params.prefix, "@")) {
            kind = ast::UnresolvedIdent::Kind::Instance;
        }

        if (kind == ast::UnresolvedIdent::Kind::Local) {
            vector<core::NameRef> locals =
                localNamesForMethod(typechecker, params.enclosingMethod, params.queryLoc, resolved);
            similarLocals = allSimilarLocalNames(gs, locals, params.prefix);
        } else {
            auto klass = params.enclosingMethod.data(gs)->owner;
            ENFORCE(klass.exists());
            similarLocals =
                allSimilarFieldsForClass(typechecker, klass, params.queryLoc, resolved, kind, params.prefix);
        }
    }

    // ----- kwargs -----

    vector<core::NameRef> similarKwargs;
    if (params.kwargsMethod.exists()) {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".determine_kwargs");

        // Find the kwargs already passed into this send
        vector<core::NameRef> existing;
        if (params.funLoc.exists()) {
            existing = KwargsFinder::findKwargs(gs, resolved, params.funLoc);
        }

        vector<core::NameRef> allKwargs;
        allKwargs.reserve(params.kwargsMethod.data(gs)->parameters.size());
        for (auto &param : params.kwargsMethod.data(gs)->parameters) {
            if (param.flags.isKeyword && !param.flags.isRepeated && hasSimilarName(gs, param.name, params.prefix)) {
                allKwargs.emplace_back(param.name);
            }
        }

        auto compareNames = +[](core::NameRef l, core::NameRef r) -> bool { return l.rawId() < r.rawId(); };

        if (!existing.empty()) {
            fast_sort(existing, compareNames);
            fast_sort(allKwargs, compareNames);
            absl::c_set_difference(allKwargs, existing, back_inserter(similarKwargs), compareNames);
        } else {
            similarKwargs = move(allKwargs);
        }
    }

    // ----- keywords -----

    auto similarKeywords = params.suggestKeywords ? allSimilarKeywords(params.prefix) : vector<RubyKeyword>{};
    auto similarLikeKeywords =
        params.suggestKeywords ? allSimilarLikeKeywords(params.prefix) : vector<KeywordLikeSnippet>{};

    // ----- methods -----

    MethodResults methodResults;
    bool receiverIsUntyped = false;

    if (params.forMethods != nullopt) {
        auto &forMethods = params.forMethods.value();
        if (forMethods.dispatchResult->main.receiver.isUntyped()) {
            receiverIsUntyped = true;
        } else {
            ENFORCE(!forMethods.dispatchResult->main.receiver.isUntyped());
            auto similarMethodsByName = allSimilarMethods(gs, *forMethods.dispatchResult, params.prefix);
            methodResults = computeDedupedMethods(gs, similarMethodsByName, forMethods.isPrivateOk, params.prefix);
        }
    }

    // ----- final sort -----

    // TODO(jez) Do something smarter here than "all keywords then all kwargs then all locals then all methods then all
    // constants"

    vector<unique_ptr<CompletionItem>> items;
    {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".keyword_items");
        for (auto &similarKeyword : similarKeywords) {
            items.push_back(getCompletionItemForKeyword(gs, this->config, similarKeyword, params.queryLoc,
                                                        params.prefix, items.size()));
        }
    }
    {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".kwarg_items");
        for (auto &similarKwarg : similarKwargs) {
            items.push_back(getCompletionItemForKwarg(gs, this->config, similarKwarg, params.queryLoc, params.prefix,
                                                      items.size()));
        }
    }
    {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".local_items");
        for (auto &similarLocal : similarLocals) {
            items.push_back(getCompletionItemForLocalName(gs, this->config, similarLocal, params.queryLoc,
                                                          params.prefix, items.size()));
        }
    }
    // since these are not actually keywords, clashing local names are valid, and we prefer those
    // hence, this is placed after local names
    {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".like_keyword_items");
        for (auto &similarLikeKeywords : similarLikeKeywords) {
            items.push_back(getCompletionItemForKeyword(gs, this->config, similarLikeKeywords, params.queryLoc,
                                                        params.prefix, items.size()));
        }
    }
    {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".method_items");
        if (receiverIsUntyped) {
            items.push_back(getCompletionItemForUntyped(gs, params.queryLoc, items.size(), "(call site is T.untyped)"));
        } else {
            for (auto &similarMethod : methodResults.methods) {
                items.push_back(getCompletionItemForMethod(typechecker, params, similarMethod, params.queryLoc,
                                                           resolved, params.prefix, items.size(),
                                                           params.forMethods->totalArgs));
            }
            for (auto &similarMethod : methodResults.operators) {
                items.push_back(getCompletionItemForMethod(typechecker, params, similarMethod, params.queryLoc,
                                                           resolved, params.prefix, items.size(),
                                                           params.forMethods->totalArgs));
            }
        }
    }

    if (!params.scopes.empty()) {
        Timer timeit(gs.tracer(), LSP_COMPLETION_METRICS_PREFIX ".constant_items");
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
    const auto &uri = params->textDocument->uri;
    auto fref = config.uri2FileRef(gs, uri);
    if (!fref.exists()) {
        response->result = std::move(emptyResult);
        return response;
    }
    auto pos = *params->position;
    auto maybeQueryLoc = pos.toLoc(gs, fref);
    if (!maybeQueryLoc.has_value()) {
        response->result = std::move(emptyResult);
        return response;
    }
    auto queryLoc = maybeQueryLoc.value();

    auto result = LSPQuery::byLoc(config, typechecker, uri, pos, LSPMethod::TextDocumentCompletion, false);

    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    auto &queryResponses = result.responses;
    vector<unique_ptr<CompletionItem>> items;
    if (queryResponses.empty()) {
        auto prevTwoChars = queryLoc.adjust(gs, -2, 0);
        if (prevTwoChars.source(gs) == "##") {
            // This is the only path that needs the resolved tree when the query response is empty, so to avoid
            // resolving the tree for the other cases we explicitly request it here.
            auto resolved = typechecker.getResolved(queryLoc.file());
            if (resolved.tree == nullptr) {
                return response;
            }

            auto item = trySuggestYardSnippet(typechecker, config.getClientConfig(), queryLoc, resolved);
            if (item != nullptr) {
                items.emplace_back(std::move(item));
                response->result = make_unique<CompletionList>(false, move(items));
                return response;
            }
        }

        auto enableTypedFalseCompletionNudges = config.getClientConfig().enableTypedFalseCompletionNudges;
        if (enableTypedFalseCompletionNudges) {
            ENFORCE(fref.exists());
            auto level = fref.data(gs).strictLevel;
            if (!fref.data(gs).hasIndexErrors() && level < core::StrictLevel::True) {
                items.emplace_back(
                    getCompletionItemForUntyped(gs, queryLoc, 0, "(file is not `# typed: true` or higher)"));
                response->result = make_unique<CompletionList>(false, move(items));
                return response;
            }
        }

        response->result = std::move(emptyResult);
        return response;
    }

    auto resp = move(queryResponses[0]);

    // All of the remaining cases require a resolved tree, so we eagerly request it here.
    auto resolved = typechecker.getResolved(queryLoc.file());
    if (resolved.tree == nullptr) {
        return response;
    }

    if (auto sendResp = resp->isSend()) {
        auto callerSideName = sendResp->callerSideName;
        auto funLoc = sendResp->funLoc();
        auto prefix =
            (callerSideName == core::Names::methodNameMissing() || (funLoc.exists() && !funLoc.contains(queryLoc)))
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
                nullopt,            // do not suggest methods
                false,              // do not suggest keywords
                core::MethodRef{},  // do not suggest locals
                core::MethodRef{},  // do not suggest kwargs
                core::LocOffsets{}, // no kwargs send loc available
                core::LocOffsets{}, // no receiver loc available
                move(scopes),
            };
            items = this->getCompletionItems(typechecker, params, resolved);
        } else {
            // isPrivateOk indicates that we are calling (and therefore completing) a method on `self`.  In such a
            // case, it matters whether or not there is a syntactic receiver involved in the call.  In the latter
            // case, we want to include locals and keywords, since we may be completing a (zero-argument) "send"
            // and the user's intent might have been to complete a local or a keyword.  In the former case, we
            // know that the user doesn't want such completion results, since they have already written something
            // prefixed with `self.`.
            auto explicitSelfReceiver = sendResp->receiverLoc().source(gs) == "self";
            auto wantLocalsAndKeywords = sendResp->isPrivateOk && !explicitSelfReceiver;
            auto suggestKeywords = wantLocalsAndKeywords;
            // `enclosingMethod` existing indicates whether we want local variable completion results.
            auto enclosingMethod = wantLocalsAndKeywords ? sendResp->enclosingMethod : core::MethodRef{};

            EnclosingSend kwargsSend;
            if (prefix.empty()) {
                // If the location of the method doesn't include our query, we can assume that we're inside of the
                // argument list.
                if (!sendResp->funLocOffsets.contains(queryLoc.offsets())) {
                    kwargsSend.funLoc = funLoc.offsets();
                    kwargsSend.method = sendResp->dispatchResult->main.method;
                }
            } else if (queryResponses.size() > 1) {
                // We only want to enable kwarg completion if completion is requested at the end of the pattern.
                auto fullPrefix = queryLoc.adjust(gs, -1 * static_cast<int32_t>(prefix.size()), 0);
                if (fullPrefix.exists() && fullPrefix.source(gs) == prefix) {
                    // As we know the prefix is non-empty, that means the first query response will have been for things
                    // related to the query prefix. If there's a second query response that's also a send result, that
                    // means we're inside the args list of another send, and we can add in kwargs for that method to the
                    // list.
                    kwargsSend = findEnclosingSend(gs, absl::MakeSpan(queryResponses).subspan(1));
                }
            }

            auto params = SearchParams{
                queryLoc,
                prefix,
                MethodSearchParams{
                    sendResp->dispatchResult,
                    sendResp->argLocOffsets.size(),
                    sendResp->isPrivateOk,
                },
                suggestKeywords,
                enclosingMethod,
                kwargsSend.method,
                kwargsSend.funLoc,
                sendResp->receiverLocOffsets,
                core::lsp::ConstantResponse::Scopes{}, // constants don't make sense here
            };
            items = this->getCompletionItems(typechecker, params, resolved);
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
                if (!behindCursor.exists() || behindCursor.source(gs) != prefix) {
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
                queryLoc,
                prefix,
                methodSearchParams,
                suggestKeywords,
                enclosingMethod,
                core::MethodRef{},  // do not suggest kwargs
                core::LocOffsets{}, // no kwargs send available
                core::LocOffsets{}, // we could pass the scope loc here, but we aren't yet using this for constants
                move(constantResp->scopes),
            };
        }
        items = this->getCompletionItems(typechecker, params, resolved);
    } else if (auto identResp = resp->isIdent()) {
        auto varName = identResp->variable._name;
        auto prefix = varName.shortName(gs);
        switch (varName.rawId()) {
            case core::Names::ivarNameMissing().rawId():
                prefix = "@";
                break;
            case core::Names::cvarNameMissing().rawId():
                prefix = "@@";
                break;
        }
        auto nameLen = static_cast<int32_t>(prefix.size());

        auto termLocPrefix = identResp->termLoc.adjustLen(gs, 0, nameLen);

        if (queryLoc.adjustLen(gs, -1 * nameLen, nameLen).source(gs) == prefix) {
            auto kwargsMethod = findEnclosingSend(gs, absl::MakeSpan(queryResponses).subspan(1));
            // Cursor at end of variable name
            auto suggestKeywords = true;
            auto params = SearchParams{
                queryLoc,
                prefix,
                methodSearchParamsForEmptyAssign(gs, identResp->enclosingMethod),
                suggestKeywords,
                identResp->enclosingMethod,
                kwargsMethod.method,
                kwargsMethod.funLoc,
                core::LocOffsets{}, // no reciever loc
                core::lsp::ConstantResponse::Scopes{identResp->enclosingMethod.data(gs)->owner},
            };
            items = this->getCompletionItems(typechecker, params, resolved);
        } else if (termLocPrefix.source(gs) == prefix && !termLocPrefix.contains(queryLoc)) {
            // This is *probably* (but not definitely necessarily) an IdentResponse for an
            // assignment, with the cursor somewhere on the RHS of the `=` but before having typed
            // anything. This case is super common for code like this (cursor is `|`):
            //     x =|
            //     y = nil
            // This technically parses and comes back as an IdentResponse for the whole `x =`
            // assignment. Let's just toss that away and suggest with an empty prefix.
            auto params = searchParamsForEmptyAssign(gs, queryLoc, identResp->enclosingMethod, {});
            items = this->getCompletionItems(typechecker, params, resolved);
        }
    } else if (auto *methodResp = resp->isMethodDef()) {
        if (!methodResp->symbol.data(gs)->hasSig() && !methodResp->isAttrBestEffortUIOnly) {
            auto owner = methodResp->symbol.data(gs)->owner;
            auto prefix = methodResp->name.shortName(gs);
            auto similarMethodsByName = similarMethodsForClass(gs, owner, prefix);
            auto similarMethods = computeDedupedMethods(gs, similarMethodsByName, /* isPrivateOk */ true, prefix);
            for (auto &similarMethod : similarMethods.methods) {
                const auto &data = similarMethod.method.data(gs);
                if (data->name == methodResp->name || data->flags.isOverloaded || data->owner == owner) {
                    continue;
                }

                string abstractOrOverridable;
                if (data->flags.isAbstract) {
                    abstractOrOverridable = "abstract";
                } else if (data->flags.isOverridable) {
                    abstractOrOverridable = "overridable";
                } else {
                    continue;
                }

                auto item = make_unique<CompletionItem>(fmt::format("def {}", data->name.shortName(gs)));
                item->kind = CompletionItemKind::Snippet; // just the icon, not whether it actually has tabstops
                item->detail = fmt::format("{} inherited method", abstractOrOverridable);

                auto showOptions = core::ShowOptions().withUseValidSyntax().withConcretizeIfAbstractOrOverridable();
                if (owner.data(gs)->attachedClass(gs).exists()) {
                    showOptions = showOptions.withForceSelfPrefix();
                }
                auto methodDefinition =
                    core::source_generator::prettyTypeForMethod(gs, similarMethod.method, nullptr, showOptions);

                if (config.getClientConfig().clientCompletionItemSnippetSupport) {
                    methodDefinition = absl::StrReplaceAll(methodDefinition, {{"; end", "\n  ${0}\nend"}});
                    item->insertTextFormat = InsertTextFormat::Snippet;
                } else {
                    item->insertTextFormat = InsertTextFormat::PlainText;
                }
                item->textEdit = make_unique<TextEdit>(Range::fromLoc(gs, methodResp->declLoc()), methodDefinition);

                items.emplace_back(move(item));
            }
        }
    }

    response->result = make_unique<CompletionList>(false, move(items));
    return response;
}

} // namespace sorbet::realmain::lsp
