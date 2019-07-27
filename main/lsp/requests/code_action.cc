#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
LSPResult LSPLoop::handleTextDocumentCodeAction(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                const TextDocumentCodeActionParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    vector<unique_ptr<CodeAction>> result;

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.codeAction");

    core::FileRef file = uri2FileRef(params.textDocument->uri);
    vector<core::FileRef> files = {file};
    auto run = tryFastPath(move(gs), {}, files);
    for (auto &e : run.errors) {
        if (!e->isSilenced && e->loc.file() == file && !e->autocorrects.empty() &&
            loc2Range(*gs, e->loc) == params.range) {
            vector<unique_ptr<TextEdit>> edits;
            edits.reserve(e->autocorrects.size());
            for (auto &a : e->autocorrects) {
                edits.emplace_back(make_unique<TextEdit>(loc2Range(*gs, a.loc), a.replacement));
            }

            vector<unique_ptr<TextDocumentEdit>> documentEdits;
            documentEdits.emplace_back(make_unique<TextDocumentEdit>(
                make_unique<VersionedTextDocumentIdentifier>(params.textDocument->uri, JSONNullObject()), move(edits)));

            auto workspaceEdit = make_unique<WorkspaceEdit>();
            workspaceEdit->documentChanges = move(documentEdits);

            auto action = make_unique<CodeAction>(e->header);
            action->kind = CodeActionKind::Quickfix;
            action->edit = move(workspaceEdit);

            result.emplace_back(move(action));
        }
    }

    response->result = move(result);

    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
