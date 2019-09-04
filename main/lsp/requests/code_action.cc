#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
LSPResult LSPLoop::handleTextDocumentCodeAction(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                const CodeActionParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    if (!config.opts.lspQuickFixEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Quick Fix` LSP feature is experimental and disabled by default.");
        return LSPResult::make(move(gs), move(response));
    }

    vector<unique_ptr<CodeAction>> result;

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.codeAction");

    core::FileRef file = config.uri2FileRef(*gs, params.textDocument->uri);
    LSPFileUpdates updates;
    updates.canTakeFastPath = true;
    ENFORCE(file.id() < globalStateHashes.size());
    updates.updatedFileHashes = {globalStateHashes[file.id()]};
    updates.updatedFiles.push_back(make_shared<core::File>(string(file.data(*gs).path()),
                                                           string(file.data(*gs).source()), core::File::Type::Normal));
    // Simply querying the file in question is insufficient since indexing errors would not be detected.
    auto run = runTypechecking(move(gs), move(updates));

    auto loc = params.range->toLoc(*run.gs, file);
    for (auto &error : run.errors) {
        if (!error->isSilenced && !error->autocorrects.empty()) {
            // We return code actions corresponding to any error that encloses the request's range. Matching request
            // ranges against error ranges exactly prevents VSCode's quick fix shortcut (Cmd+.) from matching any
            // actions since it sends a 0 length range (i.e. the cursor). VSCode's request does include matching
            // diagnostics in the request context that could be used instead but this simpler approach should suffice
            // until proven otherwise.
            if (!error->loc.contains(loc)) {
                continue;
            }

            for (auto &autocorrect : error->autocorrects) {
                UnorderedMap<string, vector<unique_ptr<TextEdit>>> editsByFile;
                for (auto &edit : autocorrect.edits) {
                    auto range = Range::fromLoc(*run.gs, edit.loc);
                    if (range != nullptr) {
                        editsByFile[config.fileRef2Uri(*run.gs, edit.loc.file())].emplace_back(
                            make_unique<TextEdit>(std::move(range), edit.replacement));
                    }
                }

                vector<unique_ptr<TextDocumentEdit>> documentEdits;
                for (auto &it : editsByFile) {
                    // TODO: Document version
                    documentEdits.emplace_back(make_unique<TextDocumentEdit>(
                        make_unique<VersionedTextDocumentIdentifier>(it.first, JSONNullObject()), move(it.second)));
                }

                auto workspaceEdit = make_unique<WorkspaceEdit>();
                workspaceEdit->documentChanges = move(documentEdits);

                auto action = make_unique<CodeAction>(autocorrect.title);
                action->kind = CodeActionKind::Quickfix;
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

    response->result = move(result);

    return LSPResult::make(move(run.gs), move(response));
}
} // namespace sorbet::realmain::lsp
