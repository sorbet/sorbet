#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

UnorderedMap<core::NameRef, vector<core::SymbolRef>>
mergeMaps(UnorderedMap<core::NameRef, vector<core::SymbolRef>> &&first,
          UnorderedMap<core::NameRef, vector<core::SymbolRef>> &&second) {
    for (auto &other : second) {
        first[other.first].insert(first[other.first].end(), make_move_iterator(other.second.begin()),
                                  make_move_iterator(other.second.end()));
    }
    return move(first);
};

UnorderedMap<core::NameRef, vector<core::SymbolRef>> LSPLoop::findSimilarMethodsIn(shared_ptr<core::Type> receiver,
                                                                                   string_view name) {
    UnorderedMap<core::NameRef, vector<core::SymbolRef>> result;
    typecase(receiver.get(),
             [&](core::ClassType *c) {
                 const auto &owner = c->symbol.data(*finalGs);
                 for (auto member : owner->membersStableOrderSlow(*finalGs)) {
                     auto sym = member.second;
                     if (sym.data(*finalGs)->isMethod() && hasSimilarName(*finalGs, sym.data(*finalGs)->name, name)) {
                         result[sym.data(*finalGs)->name].emplace_back(sym);
                     }
                 }
                 for (auto mixin : owner->mixins()) {
                     result = mergeMaps(move(result), findSimilarMethodsIn(make_shared<core::ClassType>(mixin), name));
                 }
                 if (owner->superClass.exists()) {
                     result = mergeMaps(move(result),
                                        findSimilarMethodsIn(make_shared<core::ClassType>(owner->superClass), name));
                 }
             },
             [&](core::AndType *c) {
                 result = mergeMaps(findSimilarMethodsIn(c->left, name), findSimilarMethodsIn(c->right, name));
             },
             [&](core::OrType *c) {
                 auto lhs = findSimilarMethodsIn(c->left, name);
                 auto rhs = findSimilarMethodsIn(c->right, name);
                 for (auto it = rhs.begin(); it != rhs.end(); /*nothing*/) {
                     auto &other = *it;
                     auto fnd = lhs.find(other.first);
                     if (fnd == lhs.end()) {
                         it = rhs.erase(it);
                     } else {
                         it->second.insert(it->second.end(), make_move_iterator(fnd->second.begin()),
                                           make_move_iterator(fnd->second.end()));
                         ++it;
                     }
                 }
             },
             [&](core::AppliedType *c) { result = findSimilarMethodsIn(make_shared<core::ClassType>(c->klass), name); },
             [&](core::ProxyType *c) { result = findSimilarMethodsIn(c->underlying(), name); }, [&](core::Type *c) {});
    return result;
}

string LSPLoop::methodSnippet(core::GlobalState &gs, core::SymbolRef method) {
    string ret;

    auto shortName = method.data(gs)->name.data(gs)->shortName(gs);
    vector<string> typeAndArgNames;

    int i = 1;
    if (method.data(gs)->isMethod()) {
        for (auto &argSym : method.data(gs)->arguments()) {
            string s;
            if (argSym.data(gs)->isKeyword()) {
                absl::StrAppend(&s, argSym.data(gs)->name.data(gs)->shortName(gs), ": ");
            }
            absl::StrAppend(&s, "${", i++, "}");
            typeAndArgNames.emplace_back(s);
        }
    }

    return fmt::format("{}({}){}", shortName, fmt::join(typeAndArgNames.begin(), typeAndArgNames.end(), ", "), "${0}");
}

unique_ptr<string> findDocumentation(string_view sourceCode, int beginIndex) {
    // everything in the file before the method definition.
    auto preDefinition = sourceCode.substr(0, sourceCode.rfind('\n', beginIndex));

    int last_newline_loc = preDefinition.rfind('\n');
    // if there is no '\n' in preDefinition, we're at the top of the file.
    if (last_newline_loc == preDefinition.npos) {
        return nullptr;
    }
    auto prevLine = preDefinition.substr(last_newline_loc, preDefinition.size() - last_newline_loc);
    if (prevLine.find('#') == prevLine.npos) {
        return nullptr;
    }

    string documentation = "";
    // keep looking for previous newline locations, searching for lines with # in them.
    while (prevLine.find('#') != prevLine.npos) {
        documentation = absl::StrCat(prevLine.substr(prevLine.find('#') + 1, prevLine.size()), "\n", documentation);
        int prev_newline_loc = preDefinition.rfind('\n', last_newline_loc - 1);
        // if there is no '\n', we're at the top of the file, so just return documentation.
        if (prev_newline_loc == preDefinition.npos) {
            break;
        }
        prevLine = preDefinition.substr(prev_newline_loc, last_newline_loc - prev_newline_loc);
        last_newline_loc = prev_newline_loc;
    }
    if (documentation.empty()) {
        return nullptr;
    }
    return make_unique<string>(documentation);
}

void LSPLoop::addCompletionItem(rapidjson::Value &items, core::SymbolRef what, const core::QueryResponse &resp) {
    ENFORCE(what.exists());
    rapidjson::Value item;
    item.SetObject();
    item.AddMember("label", string(what.data(*finalGs)->name.data(*finalGs)->shortName(*finalGs)), alloc);
    auto resultType = what.data(*finalGs)->resultType;
    if (!resultType) {
        resultType = core::Types::untypedUntracked();
    }
    if (what.data(*finalGs)->isMethod()) {
        item.AddMember("kind", 3, alloc); // Function
        if (what.exists()) {
            item.AddMember("detail", methodDetail(what, resp.receiver.type, nullptr, resp.constraint), alloc);
        }
        item.AddMember("insertTextFormat", 2, alloc); // Snippet
        item.AddMember("insertText", methodSnippet(*finalGs, what), alloc);

        unique_ptr<string> documentation = nullptr;
        if (what.data(*finalGs)->loc().file().exists()) {
            documentation = findDocumentation(what.data(*finalGs)->loc().file().data(*finalGs).source(),
                                              what.data(*finalGs)->loc().beginPos());
        }
        if (documentation) {
            if (documentation->find("@deprecated") != documentation->npos) {
                item.AddMember("deprecated", true, alloc);
            }
            item.AddMember("documentation", move(*documentation), alloc);
        }

    } else if (what.data(*finalGs)->isStaticField()) {
        item.AddMember("kind", 21, alloc); // Constant
        item.AddMember("detail", resultType->show(*finalGs), alloc);
    } else if (what.data(*finalGs)->isClass()) {
        item.AddMember("kind", 7, alloc); // Class
    }
    items.PushBack(move(item), alloc);
}

void LSPLoop::handleTextDocumentCompletion(rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.completion");
    result.SetObject();
    result.AddMember("isIncomplete", "false", alloc);
    rapidjson::Value items;
    items.SetArray();

    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentCompletion(), false)) {
        auto queryResponses = errorQueue->drainQueryResponses();
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);

            auto receiverType = resp->receiver.type;
            if (resp->kind == core::QueryResponse::Kind::SEND) {
                auto pattern = resp->name.data(*finalGs)->shortName(*finalGs);
                logger->debug("Looking for method similar to {}", pattern);
                UnorderedMap<core::NameRef, vector<core::SymbolRef>> methods =
                    findSimilarMethodsIn(receiverType, pattern);
                vector<pair<core::NameRef, vector<core::SymbolRef>>> methodsSorted;
                methodsSorted.insert(methodsSorted.begin(), make_move_iterator(methods.begin()),
                                     make_move_iterator(methods.end()));
                absl::c_sort(methodsSorted, [&](auto leftPair, auto rightPair) -> bool {
                    return leftPair.first.data(*finalGs)->shortName(*finalGs) <
                           rightPair.first.data(*finalGs)->shortName(*finalGs);
                });
                for (auto &entry : methodsSorted) {
                    if (entry.second[0].exists()) {
                        absl::c_sort(entry.second, [&](auto lhs, auto rhs) -> bool { return lhs._id < rhs._id; });
                        addCompletionItem(items, entry.second[0], *resp);
                    }
                }
            } else if (resp->kind == core::QueryResponse::Kind::IDENT ||
                       resp->kind == core::QueryResponse::Kind::CONSTANT) {
                if (auto c = core::cast_type<core::ClassType>(receiverType.get())) {
                    auto pattern = c->symbol.data(*finalGs)->name.data(*finalGs)->shortName(*finalGs);
                    logger->debug("Looking for constant similar to {}", pattern);
                    core::SymbolRef owner = c->symbol;
                    do {
                        owner = owner.data(*finalGs)->owner;
                        for (auto member : owner.data(*finalGs)->membersStableOrderSlow(*finalGs)) {
                            auto sym = member.second;
                            if (sym.exists() &&
                                (sym.data(*finalGs)->isClass() || sym.data(*finalGs)->isStaticField()) &&
                                sym.data(*finalGs)->name.data(*finalGs)->kind == core::NameKind::CONSTANT &&
                                // hide singletons
                                hasSimilarName(*finalGs, sym.data(*finalGs)->name, pattern) &&
                                !sym.data(*finalGs)->derivesFrom(*finalGs, core::Symbols::StubClass())) {
                                addCompletionItem(items, sym, *resp);
                            }
                        }
                    } while (owner != core::Symbols::root());
                }
            } else {
            }
        }
        result.AddMember("items", move(items), alloc);
    }
    sendResult(d, result);
}

} // namespace sorbet::realmain::lsp