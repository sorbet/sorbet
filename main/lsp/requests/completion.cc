#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "common/typecase.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

struct RubyKeyword {
    const string keyword;
    const string documentation;

    RubyKeyword(string keyword, string documentation) : keyword(keyword), documentation(documentation){};
};

// Taken from https://docs.ruby-lang.org/en/2.6.0/keywords_rdoc.html
// We might want to put this somewhere shareable if there are more places that want to use it.
const RubyKeyword rubyKeywords[] = {
    {"BEGIN", "Runs before any other code in the current file."},
    {"END", "Runs after any other code in the current file."},
    {"__ENCODING__", "The script encoding of the current file."},
    {"__FILE__", "The path to the current file."},
    {"__LINE__", "The line number of this keyword in the current file."},
    {"alias", "Creates an alias between two methods (and other things)."},
    {"and", "Short-circuit Boolean and with lower precedence than &&"},
    {"begin", "Starts an exception handling block."},
    {"break", "Leaves a block early."},
    {"case", "Starts a case expression."},
    {"class", "Creates or opens a class."},
    {"def", "Defines a method."},
    {"defined?", "Returns a string describing its argument."},
    {"do", "Starts a block."},
    {"else", "The unhandled condition in case, if and unless expressions."},
    {"elsif", "An alternate condition for an if expression."},
    {"end",
     "The end of a syntax block. Used by classes, modules, methods, exception handling and control expressions."},
    {"ensure", "Starts a section of code that is always run when an exception is raised."},
    {"false", "Boolean false."},
    {"for", "A loop that is similar to using the each method."},
    {"if", "Used for if and modifier if expressions."},
    {"in", "Used to separate the iterable object and iterator variable in a for loop."},
    {"module", "Creates or opens a module."},
    {"next", "Skips the rest of the block."},
    {"nil", "A false value usually indicating “no value” or “unknown”."},
    {"not", "Inverts the following boolean expression. Has a lower precedence than !"},
    {"or", "Boolean or with lower precedence than ||"},
    {"redo", "Restarts execution in the current block."},
    {"rescue", "Starts an exception section of code in a begin block."},
    {"retry", "Retries an exception block."},
    {"return", "Exits a method."},
    {"self", "The object the current method is attached to."},
    {"super", "Calls the current method in a superclass."},
    {"then", "Indicates the end of conditional blocks in control structures."},
    {"true", "Boolean true."},
    // This is also defined on Kernel
    // {"undef", "Prevents a class or module from responding to a method call."},
    {"unless", "Used for unless and modifier unless expressions."},
    {"until", "Creates a loop that executes until the condition is true."},
    {"when", "A condition in a case expression."},
    {"while", "Creates a loop that executes while the condition is true."},
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

string methodSnippet(const core::GlobalState &gs, core::SymbolRef method) {
    auto shortName = method.data(gs)->name.data(gs)->shortName(gs);
    vector<string> typeAndArgNames;

    int i = 1;
    if (method.data(gs)->isMethod()) {
        for (auto &argSym : method.data(gs)->arguments()) {
            string s;
            if (argSym.flags.isBlock) {
                continue;
            }
            if (argSym.flags.isKeyword) {
                absl::StrAppend(&s, argSym.name.data(gs)->shortName(gs), ": ");
            }
            if (argSym.type) {
                absl::StrAppend(&s, "${", i++, ":", argSym.type->show(gs), "}");
            } else {
                absl::StrAppend(&s, "${", i++, "}");
            }
            typeAndArgNames.emplace_back(s);
        }
    }

    if (typeAndArgNames.empty()) {
        return fmt::format("{}{}", shortName, "${0}");
    } else {
        return fmt::format("{}({}){}", shortName, fmt::join(typeAndArgNames, ", "), "${0}");
    }
}

unique_ptr<CompletionItem> getCompletionItemForKeyword(const core::GlobalState &gs, const LSPConfiguration &config,
                                                       const RubyKeyword &rubyKeyword, const core::Loc queryLoc,
                                                       string_view prefix, size_t sortIdx) {
    auto label = rubyKeyword.keyword;
    auto item = make_unique<CompletionItem>(label);
    item->sortText = fmt::format("{:06d}", sortIdx);
    item->kind = CompletionItemKind::Keyword;

    // TODO(jez) This should probably be a helper function (see getCompletionItemForSymbol)
    u4 queryStart = queryLoc.beginPos();
    u4 prefixSize = prefix.size();
    auto replacementLoc = core::Loc{queryLoc.file(), queryStart - prefixSize, queryStart};
    auto replacementRange = Range::fromLoc(gs, replacementLoc);
    auto replacementText = label;
    if (replacementRange != nullptr) {
        item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
    } else {
        // TODO(jez) Why is replacementRange nullptr? instrument this and investigate when it fails
        item->insertText = replacementText;
    }
    item->insertTextFormat = InsertTextFormat::PlainText;

    item->documentation =
        make_unique<MarkupContent>(config.getClientConfig().clientCompletionItemMarkupKind, rubyKeyword.documentation);

    return item;
}

} // namespace

unique_ptr<CompletionItem> LSPLoop::getCompletionItemForSymbol(const core::GlobalState &gs, core::SymbolRef what,
                                                               core::TypePtr receiverType,
                                                               const core::TypeConstraint *constraint,
                                                               const core::Loc queryLoc, string_view prefix,
                                                               size_t sortIdx) const {
    ENFORCE(what.exists());
    auto item = make_unique<CompletionItem>(string(what.data(gs)->name.data(gs)->shortName(gs)));

    // Completion items are sorted by sortText if present, or label if not. We unconditionally use an index to sort.
    // If we ever have 100,000+ items in the completion list, we'll need to bump the padding here.
    item->sortText = fmt::format("{:06d}", sortIdx);

    auto resultType = what.data(gs)->resultType;
    if (!resultType) {
        resultType = core::Types::untypedUntracked();
    }
    if (what.data(gs)->isMethod()) {
        item->kind = CompletionItemKind::Method;
        if (what.exists()) {
            item->detail = methodDetail(gs, what, receiverType, nullptr, constraint);
        }

        u4 queryStart = queryLoc.beginPos();
        u4 prefixSize = prefix.size();
        auto replacementLoc = core::Loc{queryLoc.file(), queryStart - prefixSize, queryStart};
        auto replacementRange = Range::fromLoc(gs, replacementLoc);

        string replacementText;
        if (config->getClientConfig().clientCompletionItemSnippetSupport) {
            item->insertTextFormat = InsertTextFormat::Snippet;
            replacementText = methodSnippet(gs, what);
        } else {
            item->insertTextFormat = InsertTextFormat::PlainText;
            replacementText = string(what.data(gs)->name.data(gs)->shortName(gs));
        }

        if (replacementRange != nullptr) {
            item->textEdit = make_unique<TextEdit>(std::move(replacementRange), replacementText);
        } else {
            // TODO(jez) Why is replacementRange nullptr? instrument this and investigate when it fails
            item->insertText = replacementText;
        }

        optional<string> documentation = nullopt;
        if (what.data(gs)->loc().file().exists()) {
            documentation =
                findDocumentation(what.data(gs)->loc().file().data(gs).source(), what.data(gs)->loc().beginPos());
        }
        if (documentation != nullopt) {
            if (documentation->find("@deprecated") != documentation->npos) {
                item->deprecated = true;
            }
            item->documentation = make_unique<MarkupContent>(config->getClientConfig().clientCompletionItemMarkupKind,
                                                             documentation.value());
        }
    } else if (what.data(gs)->isStaticField()) {
        item->kind = CompletionItemKind::Constant;
        item->detail = resultType->show(gs);
    } else if (what.data(gs)->isClassOrModule()) {
        item->kind = CompletionItemKind::Class;
    }
    return item;
}

void LSPLoop::findSimilarConstantOrIdent(const core::GlobalState &gs, const core::TypePtr receiverType,
                                         const core::Loc queryLoc, vector<unique_ptr<CompletionItem>> &items) const {
    if (auto c = core::cast_type<core::ClassType>(receiverType.get())) {
        auto pattern = c->symbol.data(gs)->name.data(gs)->shortName(gs);
        config->logger->debug("Looking for constant similar to {}", pattern);
        core::SymbolRef owner = c->symbol;
        do {
            owner = owner.data(gs)->owner;
            for (auto member : owner.data(gs)->membersStableOrderSlow(gs)) {
                auto sym = member.second;
                if (sym.exists() && (sym.data(gs)->isClassOrModule() || sym.data(gs)->isStaticField()) &&
                    sym.data(gs)->name.data(gs)->kind == core::NameKind::CONSTANT &&
                    // hide singletons
                    hasSimilarName(gs, sym.data(gs)->name, pattern)) {
                    items.push_back(
                        getCompletionItemForSymbol(gs, sym, receiverType, nullptr, queryLoc, pattern, items.size()));
                }
            }
        } while (owner != core::Symbols::root());
    }
}

unique_ptr<ResponseMessage> LSPLoop::handleTextDocumentCompletion(LSPTypechecker &typechecker, const MessageId &id,
                                                                  const CompletionParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCompletion);
    if (!config->opts.lspAutocompleteEnabled && !config->opts.lspAutocompleteMethodsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Autocomplete` LSP feature is experimental and disabled by default.");
        return response;
    }

    auto emptyResult = make_unique<CompletionList>(false, vector<unique_ptr<CompletionItem>>{});

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.completion");

    const core::GlobalState &gs = typechecker.state();
    auto uri = params.textDocument->uri;
    auto fref = config->uri2FileRef(gs, uri);
    if (!fref.exists()) {
        response->result = std::move(emptyResult);
        return response;
    }
    auto pos = *params.position;
    auto queryLoc = config->lspPos2Loc(fref, pos, gs);
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
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);

        if (auto sendResp = resp->isSend()) {
            auto prefix = sendResp->callerSideName.data(gs)->shortName(gs);
            config->logger->debug("Looking for method similar to {}", prefix);

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

            // TODO(jez) Do something smarter here than "all matching keywords always come first"
            for (auto &similarKeyword : similarKeywords) {
                items.push_back(
                    getCompletionItemForKeyword(gs, *config, similarKeyword, queryLoc, prefix, items.size()));
            }
            for (auto &similarMethod : deduped) {
                items.push_back(getCompletionItemForSymbol(gs, similarMethod.method, similarMethod.receiverType,
                                                           similarMethod.constr.get(), queryLoc, prefix, items.size()));
            }
        } else if (auto constantResp = resp->isConstant()) {
            if (!config->opts.lspAutocompleteEnabled) {
                response->result = std::move(emptyResult);
                return response;
            }
            findSimilarConstantOrIdent(gs, constantResp->retType.type, queryLoc, items);
        }
    }

    response->result = make_unique<CompletionList>(false, move(items));
    return response;
}

} // namespace sorbet::realmain::lsp
