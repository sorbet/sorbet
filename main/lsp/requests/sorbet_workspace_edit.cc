#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

string readFile(string_view path, const FileSystem &fs) {
    try {
        return fs.readFile(path);
    } catch (FileNotFoundException e) {
        // Act as if file is completely empty.
        return "";
    }
}

string getFileContents(UnorderedMap<string, string> &updates, unique_ptr<core::GlobalState> &initialGS,
                       const string &path) {
    if (updates.find(path) != updates.end()) {
        return updates[path];
    }

    auto currentFileRef = initialGS->findFileByPath(path);
    if (currentFileRef.exists()) {
        return string(currentFileRef.data(*initialGS).source());
    } else {
        return "";
    }
}

unique_ptr<core::GlobalState> LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                                                 unique_ptr<SorbetWorkspaceEdit> &edit) {
    vector<unique_ptr<SorbetWorkspaceEdit>> edits;
    edits.push_back(move(edit));
    return handleSorbetWorkspaceEdits(move(gs), edits);
}

unique_ptr<core::GlobalState> LSPLoop::handleSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                                                  vector<unique_ptr<SorbetWorkspaceEdit>> &edits) {
    // path => new file contents
    UnorderedMap<string, string> updates;
    for (auto &edit : edits) {
        switch (edit->type) {
            case SorbetWorkspaceEditType::EditorOpen: {
                auto &openParams = get<unique_ptr<DidOpenTextDocumentParams>>(edit->contents);
                string_view uri = openParams->textDocument->uri;
                if (absl::StartsWith(uri, rootUri)) {
                    string localPath = remoteName2Local(uri);
                    if (FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns,
                                               opts.relativeIgnorePatterns)) {
                        continue;
                    }
                    openFiles.insert(localPath);
                    updates[localPath] = move(openParams->textDocument->text);
                }
                break;
            }
            case SorbetWorkspaceEditType::EditorChange: {
                auto &changeParams = get<unique_ptr<DidChangeTextDocumentParams>>(edit->contents);
                string_view uri = changeParams->textDocument->uri;
                if (absl::StartsWith(uri, rootUri)) {
                    string localPath = remoteName2Local(uri);
                    if (FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns,
                                               opts.relativeIgnorePatterns)) {
                        continue;
                    }
                    string fileContents = getFileContents(updates, initialGS, localPath);
                    for (auto &change : changeParams->contentChanges) {
                        if (change->range) {
                            auto &range = *change->range;
                            // incremental update
                            core::Loc::Detail start, end;
                            start.line = range->start->line + 1;
                            start.column = range->start->character + 1;
                            end.line = range->end->line + 1;
                            end.column = range->end->character + 1;
                            core::File old(string(localPath), string(fileContents), core::File::Type::Normal);
                            auto startOffset = core::Loc::pos2Offset(old, start);
                            auto endOffset = core::Loc::pos2Offset(old, end);
                            fileContents = fileContents.replace(startOffset, endOffset - startOffset, change->text);
                        } else {
                            // replace
                            fileContents = change->text;
                        }
                    }
                    updates[localPath] = fileContents;
                }
                break;
            }
            case SorbetWorkspaceEditType::EditorClose: {
                auto &closeParams = get<unique_ptr<DidCloseTextDocumentParams>>(edit->contents);
                string_view uri = closeParams->textDocument->uri;
                if (absl::StartsWith(uri, rootUri)) {
                    string localPath = remoteName2Local(uri);
                    if (FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns,
                                               opts.relativeIgnorePatterns)) {
                        continue;
                    }
                    auto it = openFiles.find(localPath);
                    if (it != openFiles.end()) {
                        openFiles.erase(it);
                    }
                    // Use contents of file on disk.
                    updates[localPath] = readFile(localPath, *opts.fs);
                }
                break;
            }
            case SorbetWorkspaceEditType::FileSystem: {
                if (!initialized) {
                    // We can receive watchman updates before LSP is initialized.
                    deferredFileEdits.push_back(move(edit));
                    continue;
                }
                auto &params = get<unique_ptr<WatchmanQueryResponse>>(edit->contents);
                for (auto file : params->files) {
                    string localPath = absl::StrCat(rootPath, "/", file);
                    if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns,
                                                opts.relativeIgnorePatterns) &&
                        openFiles.find(localPath) == openFiles.end()) {
                        updates[localPath] = readFile(localPath, *opts.fs);
                    }
                }
                break;
            }
        }
    }

    if (updates.size() > 0) {
        vector<shared_ptr<core::File>> files;
        files.reserve(updates.size());
        for (auto &update : updates) {
            files.push_back(
                make_shared<core::File>(string(update.first), move(update.second), core::File::Type::Normal));
        }
        return pushDiagnostics(tryFastPath(move(gs), files));
    } else {
        return gs;
    }
}

} // namespace sorbet::realmain::lsp