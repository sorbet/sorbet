#include "main/lsp/requests/code_action.h"
#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "main/sig_finder/sig_finder.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
vector<unique_ptr<TextDocumentEdit>> getQuickfixEdits(const LSPConfiguration &config, const core::GlobalState &gs,
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

// Turns ruby_function_name__ to RubyFunctionName,
// so we can use it in a new module name
string snakeToCamelCase(string_view name) {
    string res;
    const auto originalSize = name.size();
    res.reserve(originalSize);
    bool shouldCapitalize = true;
    for (int i = 0; i < originalSize; i++) {
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

class MethodCallSiteRenamer : public AbstractRenamer {
public:
    MethodCallSiteRenamer(const core::GlobalState &gs, const LSPConfiguration &config, const string oldName,
                          const string newName)
        : AbstractRenamer(gs, config, oldName, newName) {
        const vector<string> invalidNames = {"initialize", "call"};
        for (auto name : invalidNames) {
            if (oldName == name) {
                invalid = true;
                error = fmt::format("The `{}` method cannot be extracted to a module.", oldName);
                return;
            }
        }
        // block any method not starting with /[a-zA-Z0-9_]+/. This blocks operator overloads.
        if (!isalnum(oldName[0]) && oldName[0] != '_') {
            error = fmt::format("The `{}` method cannot be extracted to a module.", oldName);
            invalid = true;
        }
    }

    ~MethodCallSiteRenamer() {}
    void rename(unique_ptr<core::lsp::QueryResponse> &response) override {
        if (invalid) {
            return;
        }
        auto loc = response->getLoc();

        // If we're renaming the exact same place twice, silently ignore it. We reach this condition when we find the
        // same method send through multiple definitions (e.g. in the case of union types)
        auto it = edits.find(loc);
        if (it != edits.end()) {
            return;
        }

        auto source = loc.source(gs);
        if (!source.has_value()) {
            return;
        }
        string newsrc;
        if (auto sendResp = response->isSend()) {
            edits[sendResp->receiverLoc] = newName;
        }
    }
    void addSymbol(const core::SymbolRef symbol) override {
        if (symbol.isMethod()) {
            addSubclassRelatedMethods(gs, symbol.asMethodRef(), getQueue());
        } else {
            getQueue()->tryEnqueue(symbol);
        }
    }
}; // CallSiteRenamer

vector<unique_ptr<TextEdit>> moveMethod(const LSPConfiguration &config, const core::GlobalState &gs,
                                        const core::lsp::DefinitionResponse *definition,
                                        LSPTypecheckerInterface &typechecker, string_view newModuleName) {
    auto moduleStart = fmt::format("module {}\n  extend T::Sig\n  ", newModuleName);
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
    vector<unique_ptr<TextEdit>> res;
    res.emplace_back(make_unique<TextEdit>(std::move(range), replacement));
    res.emplace_back(make_unique<TextEdit>(range2Loc(sigLoc), ""));
    res.emplace_back(make_unique<TextEdit>(range2Loc(methodLoc), ""));
    return res;
}
} // namespace

CodeActionTask::CodeActionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CodeActionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCodeAction), params(move(params)) {}

vector<unique_ptr<TextDocumentEdit>>
CodeActionTask::getExtractMethodEdits(const LSPConfiguration &config, const core::GlobalState &gs,
                                      const core::lsp::DefinitionResponse *definition,
                                      LSPTypecheckerInterface &typechecker) {
    ENFORCE(definition->symbol.isMethod());

    vector<unique_ptr<TextDocumentEdit>> res;
    auto newModuleName = getNewModuleName(gs, definition->name);
    if (!newModuleName.has_value()) {
        return res;
    }

    vector<unique_ptr<TextEdit>> edits = moveMethod(config, gs, definition, typechecker, newModuleName.value());

    auto renamer = make_shared<MethodCallSiteRenamer>(gs, config, definition->name.show(gs), newModuleName.value());
    getRenameEdits(typechecker, renamer, definition->symbol, newModuleName.value());
    auto callSiteEdits = renamer->buildTextDocumentEdits();

    if (callSiteEdits.has_value()) {
        for (auto &edit : callSiteEdits.value()) {
            res.emplace_back(std::move(edit));
        }
    }
    auto docEdit =
        make_unique<TextDocumentEdit>(make_unique<VersionedTextDocumentIdentifier>(
                                          config.fileRef2Uri(gs, definition->termLoc.file()), JSONNullObject()),
                                      std::move(edits));

    res.emplace_back(std::move(docEdit));
    return res;
}

unique_ptr<ResponseMessage> CodeActionTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    vector<unique_ptr<CodeAction>> result;

    if (typechecker.isStale()) {
        config.logger->debug("CodeActionTask running on stale, returning empty result");
        response->result = move(result);
        return response;
    }

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
                workspaceEdit->documentChanges = getQuickfixEdits(config, gs, autocorrect.edits);
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
            workspaceEdit->documentChanges = getQuickfixEdits(config, gs, allEdits);
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
                        workspaceEdit->documentChanges = getExtractMethodEdits(config, gs, def, typechecker);
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

bool CodeActionTask::canUseStaleData() const {
    return true;
}
} // namespace sorbet::realmain::lsp
