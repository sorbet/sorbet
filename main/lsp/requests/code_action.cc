#include "main/lsp/requests/code_action.h"
#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "main/sig_finder/sig_finder.h"

using namespace std;

namespace sorbet::realmain::lsp {

vector<unique_ptr<TextDocumentEdit>> getEdits(const LSPConfiguration &config, const core::GlobalState &gs,
                                              const vector<core::AutocorrectSuggestion::Edit> &edits) {
    UnorderedMap<string, vector<unique_ptr<TextEdit>>> editsByFile;
    for (auto &edit : edits) {
        auto range = Range::fromLoc(gs, edit.loc);
        if (range != nullptr) {
            editsByFile[config.fileRef2Uri(gs, edit.loc.file())].emplace_back(
                make_unique<TextEdit>(std::move(range), edit.replacement));
        }
    }

    vector<unique_ptr<TextDocumentEdit>> documentEdits;
    for (auto &it : editsByFile) {
        // TODO: Document version
        documentEdits.emplace_back(make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(it.first, JSONNullObject()), move(it.second)));
    }
    return documentEdits;
}

std::optional<const ast::MethodDef *> findMethodTree(const ast::ExpressionPtr &tree, const core::SymbolRef &method) {
    if (auto seq = ast::cast_tree<ast::InsSeq>(tree)) {
        for (auto &subtree : seq->stats) {
            auto maybeMethod = findMethodTree(subtree, method);
            if (maybeMethod.has_value()) {
                return maybeMethod.value();
            }
        }
    } else if (auto klass = ast::cast_tree<ast::ClassDef>(tree)) {
        for (auto &subtree : klass->rhs) {
            auto maybeMethod = findMethodTree(subtree, method);
            if (maybeMethod.has_value()) {
                return maybeMethod.value();
            }
        }
    } else if (auto methodDef = ast::cast_tree<ast::MethodDef>(tree)) {
        if (methodDef->symbol == method.asMethodRef()) {
            return methodDef;
        }
    }
    return std::nullopt;
}

unique_ptr<string> copyMethodSource(const core::GlobalState &gs, const core::LocOffsets &sigLoc,
                                    const core::LocOffsets &methodLoc, const core::FileRef &fref) {
    auto cutSource = [&](core::LocOffsets loc) {
        return fref.data(gs).source().substr(loc.beginPos(), loc.endPos() - loc.beginPos());
    };

    auto sigSource = cutSource(sigLoc);
    auto methodSource = cutSource(methodLoc);
    return make_unique<string>(absl::StrCat(sigSource, "\n  ", methodSource));
}

// Follow superClass links until we find the highest class that contains the given method. In other words we find the
// "root" of the tree of classes that define a method.
core::ClassOrModuleRef findRootClassWithMethod(const core::GlobalState &gs, core::ClassOrModuleRef klass,
                                               core::NameRef methodName) {
    auto root = klass;
    while (true) {
        auto tmp = root.data(gs)->superClass();
        ENFORCE(tmp.exists()); // everything derives from Kernel::Object so we can't ever reach the actual top type
        if (!tmp.exists() || !(tmp.data(gs)->findMember(gs, methodName).exists())) {
            break;
        }
        root = tmp;
    }
    return root;
}

std::optional<std::pair<core::LocOffsets, core::LocOffsets>> methodLocs(const core::GlobalState &gs,
                                                                        const ast::ExpressionPtr &rootTree,
                                                                        const core::SymbolRef &method,
                                                                        const core::FileRef &fref) {
    auto maybeTree = findMethodTree(rootTree, method.asMethodRef());
    if (!maybeTree.has_value()) {
        return std::nullopt;
    }
    auto methodLoc = maybeTree.value()->loc;

    auto maybeSig = sorbet::sig_finder::findSignature(gs, method);
    if (!maybeSig.has_value()) {
        return std::nullopt;
    }
    core::LocOffsets sigLoc = {maybeSig->sig.beginPos(), maybeSig->body.endPos()};

    return make_pair(sigLoc, methodLoc);
}

CodeActionTask::CodeActionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CodeActionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCodeAction), params(move(params)) {}

class UniqueSymbolQueue {
public:
    UniqueSymbolQueue() {}

    bool tryEnqueue(core::SymbolRef s) {
        auto insertResult = set.insert(s);
        bool isNew = insertResult.second;
        if (isNew) {
            symbols.emplace_back(s);
        }
        return isNew;
    }

    core::SymbolRef pop() {
        if (!symbols.empty()) {
            auto s = symbols.front();
            symbols.pop_front();
            return s;
        }
        return core::Symbols::noSymbol();
    }

private:
    deque<core::SymbolRef> symbols;
    UnorderedSet<core::SymbolRef> set;
};

void addSubclassRelatedMethods(const core::GlobalState &gs, core::MethodRef symbol, UniqueSymbolQueue &methods) {
    auto symbolData = symbol.data(gs);

    // We have to check for methods as part of a class hierarchy: Follow superClass() links till we find the root;
    // then find the full tree; then look for methods with the same name as ours; then find all references to all
    // those methods and rename them.
    auto symbolClass = symbol.enclosingClass(gs);

    // We have to be careful to follow superclass links only as long as we find a method that `symbol` overrides.
    // Otherwise we will find unrelated methods and rename them even though they don't need to be (see the
    // method_class_hierarchy test case for an example).
    auto root = findRootClassWithMethod(gs, symbolClass, symbolData->name);

    // Scans whole symbol table. This is slow, and we might need to make this faster eventually.
    auto includeRoot = true;
    auto subclasses = getSubclassesSlow(gs, root, includeRoot);

    // find the target method definition in each subclass
    for (auto c : subclasses) {
        auto classSymbol = c.data(gs);
        auto member = classSymbol->findMethod(gs, symbolData->name);
        if (!member.exists()) {
            continue;
        }
        methods.tryEnqueue(member);
    }
}

// Add methods that are related because of dispatching via secondary components in sends (union types).
void addDispatchRelatedMethods(const core::GlobalState &gs, const core::DispatchResult *dispatchResult,
                               UniqueSymbolQueue &methods) {
    for (const core::DispatchResult *dr = dispatchResult; dr != nullptr; dr = dr->secondary.get()) {
        auto method = dr->main.method;
        ENFORCE(method.exists());
        auto isNew = methods.tryEnqueue(method);
        if (isNew) {
            addSubclassRelatedMethods(gs, method, methods);
        }
    }
}
vector<unique_ptr<TextEdit>> CodeActionTask::updateCallSites(const core::GlobalState &gs, const core::SymbolRef symbol,
                                                             string_view newModuleName,
                                                             LSPTypecheckerDelegate &typechecker) {
    const auto originalName = symbol.name(gs).show(gs);
    UniqueSymbolQueue symbolQueue;
    addSubclassRelatedMethods(gs, symbol.asMethodRef(), symbolQueue);

    vector<unique_ptr<TextEdit>> res;
    for (auto sym = symbolQueue.pop(); sym.exists(); sym = symbolQueue.pop()) {
        auto queryResult = queryBySymbol(typechecker, sym);
        if (queryResult.error) {
            return {};
        }

        // Filter for untyped files, and deduplicate responses by location.  We don't use extractLocations here because
        // in some cases like sends, we need the SendResponse to be able to accurately find the method name in the
        // expression.
        for (auto &response : filterAndDedup(gs, queryResult.responses)) {
            const auto loc = response->getLoc();
            if (loc.file().data(gs).isPayload()) {
                // We don't support renaming things in payload files.
                return {};
            }

            // We may process the same send multiple times in case of union types, but this is ok because the renamer
            // de-duplicates edits at the same location
            UnorderedMap<core::Loc, string> edits;

            // If we're renaming the exact same place twice, silently ignore it. We reach this condition when we find
            // the same method send through multiple definitions (e.g. in the case of union types)
            auto it = edits.find(loc);
            if (it != edits.end()) {
                continue;
            }

            auto source = loc.source(gs);
            if (!source.has_value()) {
                continue;
            }
            // put method manipulatuibs here
            // ref:
            // main/lsp/requests/rename.cc:228
            if (auto sendResp = response->isSend()) {
                if (sendResp->dispatchResult->secondary) {
                    addDispatchRelatedMethods(gs, sendResp->dispatchResult.get(), symbolQueue);
                }
                auto fref = sendResp->receiverLoc.file();
                auto pos2Loc = [&](const auto &pos) {
                    auto loc = core::Loc::offset2Pos(fref.data(gs), pos);
                    return make_unique<Position>(loc.line - 1, loc.column - 1);
                };
                auto range2Loc = [&](const auto &pos) {
                    return make_unique<Range>(pos2Loc(pos.beginPos()), pos2Loc(pos.endPos()));
                };
                res.emplace_back(make_unique<TextEdit>(range2Loc(sendResp->receiverLoc), newModuleName.data()));
            }
        }
    }

    return res;
}

// Turns ruby_function_name__ to RubyFunctionName,
// so we can use it in a new module name
string snakeToCamelCase(string_view name) {
    string res;
    const auto originalSize = name.size();
    res.reserve(originalSize);
    bool shouldCapitalize = true;
    for (int i = 0; i < originalSize - 1; i++) {
        if (name.at(i) == '_') {
            shouldCapitalize = true;
            continue;
        }
        res += shouldCapitalize ? toupper(name[i]) : name[i];
        shouldCapitalize = false;
    }
    return res;
}

optional<string> getNewModuleName(const core::GlobalState &gs, const core::NameRef name) {
    const auto moduleName = absl::StrCat(snakeToCamelCase(name.show(gs)), "Module");
    const auto nameNotExists = [&](auto moduleName) {
        return gs.lookupNameUTF8(moduleName) == core::NameRef::noName();
    };

    // if there are no names like that, return the new module name
    if (nameNotExists(moduleName)) {
        return moduleName;
    }

    // otherwise, try to augment it
    for (int i = 1; i < 42; i++) {
        const auto newName = absl::StrCat(moduleName, i);
        if (nameNotExists(newName)) {
            return newName;
        }
    }

    // bail, if we haven't found untaken name in 42 tries
    return nullopt;
}

vector<unique_ptr<TextDocumentEdit>> CodeActionTask::getMoveMethodEdits(const LSPConfiguration &config,
                                                                        const core::GlobalState &gs,
                                                                        const core::lsp::DefinitionResponse *definition,
                                                                        LSPTypecheckerDelegate &typechecker) {
    ENFORCE(definition->symbol.isMethod());

    vector<unique_ptr<TextDocumentEdit>> res;
    auto newModuleName = getNewModuleName(gs, definition->name);
    if (!newModuleName.has_value()) {
        return res;
    }

    auto moduleStart = fmt::format("module {}\n  extend T::Sig\n  ", newModuleName.value());
    auto moduleEnd = "\nend";

    auto fref = definition->termLoc.file();
    auto &file = fref.data(gs);

    auto trees = typechecker.getResolved({fref});
    auto &rootTree = trees[0].tree;
    auto beginLoc = core::Loc::offset2Pos(fref.data(gs), rootTree.loc().beginPos());

    auto pos2Loc = [&](const auto &pos) {
        auto loc = core::Loc::offset2Pos(fref.data(gs), pos);
        return make_unique<Position>(loc.line - 1, loc.column - 1);
    };

    auto range2Loc = [&](const auto &pos) {
        return make_unique<Range>(pos2Loc(pos.beginPos()), pos2Loc(pos.endPos()));
    };

    auto topOfTheFile = file.getLine(beginLoc.line);
    auto sigAndMethodLocs = methodLocs(gs, rootTree, definition->symbol, fref);
    if (!sigAndMethodLocs.has_value()) {
        return {};
    }
    auto [sigLoc, methodLoc] = sigAndMethodLocs.value();
    auto methodSource = copyMethodSource(gs, sigLoc, methodLoc, fref);

    auto range = make_unique<Range>(make_unique<Position>(beginLoc.line - 1, 0),
                                    make_unique<Position>(beginLoc.line - 1, topOfTheFile.length()));
    auto replacement = fmt::format("{}{}{}\n\n{}", moduleStart, *methodSource, moduleEnd, topOfTheFile);
    auto moveNewMethod = make_unique<TextEdit>(std::move(range), replacement);
    auto deleteOldSig = make_unique<TextEdit>(range2Loc(sigLoc), "");
    auto deleteOldMethod = make_unique<TextEdit>(range2Loc(methodLoc), "");
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(std::move(moveNewMethod));
    edits.emplace_back(std::move(deleteOldSig));
    edits.emplace_back(std::move(deleteOldMethod));

    auto callSites = updateCallSites(gs, definition->symbol, newModuleName.value(), typechecker);
    for (auto &edit : callSites) {
        edits.emplace_back(std::move(edit));
    }
    auto docEdit =
        make_unique<TextDocumentEdit>(make_unique<VersionedTextDocumentIdentifier>(
                                          config.fileRef2Uri(gs, definition->termLoc.file()), JSONNullObject()),
                                      std::move(edits));

    res.emplace_back(std::move(docEdit));
    return res;
}
unique_ptr<ResponseMessage> CodeActionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    vector<unique_ptr<CodeAction>> result;

    const core::GlobalState &gs = typechecker.state();
    core::FileRef file = config.uri2FileRef(gs, params->textDocument->uri);
    if (!file.exists()) {
        // File is an invalid URI. Perhaps the user opened a file that is not within the VS Code workspace?
        // Don't send an error, as it's not the user's fault and isn't actionable. Instead, send an empty list of code
        // actions.
        response->result = move(result);
        return response;
    }

    // Simply querying the file in question is insufficient since indexing errors would not be detected.
    auto errors = typechecker.retypecheck({file});
    auto loc = params->range->toLoc(gs, file);
    vector<core::AutocorrectSuggestion::Edit> allEdits;
    for (auto &error : errors) {
        if (!error->isSilenced && !error->autocorrects.empty()) {
            // Collect all autocorrects regardless of range to compile into a "source" autocorrect whose scope is
            // the whole file.
            for (auto &autocorrect : error->autocorrects) {
                allEdits.insert(allEdits.end(), autocorrect.edits.begin(), autocorrect.edits.end());
            }

            // We return code actions corresponding to any error that encloses the request's range. Matching request
            // ranges against error ranges exactly prevents VSCode's quick fix shortcut (Cmd+.) from matching any
            // actions since it sends a 0 length range (i.e. the cursor). VSCode's request does include matching
            // diagnostics in the request context that could be used instead but this simpler approach should suffice
            // until proven otherwise.
            if (!error->loc.contains(loc)) {
                continue;
            }

            for (auto &autocorrect : error->autocorrects) {
                auto action = make_unique<CodeAction>(autocorrect.title);
                action->kind = CodeActionKind::Quickfix;
                auto workspaceEdit = make_unique<WorkspaceEdit>();
                workspaceEdit->documentChanges = getEdits(config, gs, autocorrect.edits);
                action->edit = move(workspaceEdit);
                result.emplace_back(move(action));
            }
        }
    }

    // TODO(sushain): investigate where duplicates might happen and whether there is a better fix
    // Remove any actions with the same header regardless of their actions since users cannot make an informed decision
    // between two seemingly identical actions.
    fast_sort(result, [](const auto &l, const auto &r) -> bool { return l->title.compare(r->title) > 0; });
    auto last =
        unique(result.begin(), result.end(), [](const auto &l, const auto &r) -> bool { return l->title == r->title; });
    result.erase(last, result.end());

    // Perform _after_ sorting so these appear at the end of the list.
    if (!allEdits.empty()) {
        // Make a source autocorrect that appears in the "source" menu, and an autocorrect that appears in the
        // quickfix dropdown if there are other quickfixes at this location.
        vector<CodeActionKind> kinds = {CodeActionKind::SourceFixAllSorbet};
        if (!result.empty()) {
            kinds.push_back(CodeActionKind::Quickfix);
        }

        for (auto kind : kinds) {
            auto action = make_unique<CodeAction>("Apply all Sorbet fixes for file");
            action->kind = kind;
            auto workspaceEdit = make_unique<WorkspaceEdit>();
            workspaceEdit->documentChanges = getEdits(config, gs, allEdits);
            action->edit = move(workspaceEdit);
            result.emplace_back(move(action));
        }
    }

    if (config.opts.lspExtractMethodEnabled) {
        auto queryResult = queryByLoc(typechecker, params->textDocument->uri, *params->range->start,
                                      LSPMethod::TextDocumentCodeAction, false);

        // Generate "Extract method" code actions only for method definitions
        if (queryResult.error == nullptr) {
            for (auto &resp : queryResult.responses) {
                if (auto def = resp->isDefinition()) {
                    if (def->symbol.isMethod()) {
                        auto action = make_unique<CodeAction>("Extract method to module");
                        action->kind = CodeActionKind::RefactorExtract;
                        auto workspaceEdit = make_unique<WorkspaceEdit>();
                        workspaceEdit->documentChanges = getMoveMethodEdits(config, gs, def, typechecker);
                        action->edit = move(workspaceEdit);
                        result.emplace_back(move(action));
                    }
                }
            }
        }
    }

    response->result = move(result);
    return response;
}
} // namespace sorbet::realmain::lsp
