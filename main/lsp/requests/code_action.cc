#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
unique_ptr<ResponseMessage> LSPLoop::handleTextDocumentCodeAction(LSPTypechecker &typechecker, const MessageId &id,
                                                                  const CodeActionParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    if (!config->opts.lspQuickFixEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Quick Fix` LSP feature is experimental and disabled by default.");
        return response;
    }

    vector<unique_ptr<CodeAction>> result;

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.codeAction");

    const core::GlobalState &gs = typechecker.state();
    core::FileRef file = config->uri2FileRef(gs, params.textDocument->uri);
    if (!file.exists()) {
        // File is an invalid URI. Perhaps the user opened a file that is not within the VS Code workspace?
        // Don't send an error, as it's not the user's fault and isn't actionable. Instead, send an empty list of code
        // actions.
        response->result = move(result);
        return response;
    }

    // Simply querying the file in question is insufficient since indexing errors would not be detected.
    auto run = typechecker.retypecheck({file});
    auto loc = params.range->toLoc(gs, file);
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
                    auto range = Range::fromLoc(gs, edit.loc);
                    if (range != nullptr) {
                        editsByFile[config->fileRef2Uri(gs, edit.loc.file())].emplace_back(
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
    return response;
}
} // namespace sorbet::realmain::lsp
