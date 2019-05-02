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
        // NOTE: It is not appropriate to throw an error here. Sorbet does not differentiate between Watchman updates
        // that specify if a file has changed or has been deleted, so this is the 'golden path' for deleted files.
        // TODO(jvilk): Use Tombstone files instead.
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

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidChangeTextDocumentParams &changeParams,
                                            UnorderedMap<string, string> &updates) {
    string_view uri = changeParams.textDocument->uri;
    if (absl::StartsWith(uri, rootUri)) {
        string localPath = remoteName2Local(uri);
        if (FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns)) {
            return;
        }
        string fileContents = getFileContents(updates, initialGS, localPath);
        for (auto &change : changeParams.contentChanges) {
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
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidOpenTextDocumentParams &openParams,
                                            UnorderedMap<string, string> &updates) {
    string_view uri = openParams.textDocument->uri;
    if (absl::StartsWith(uri, rootUri)) {
        string localPath = remoteName2Local(uri);
        if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns)) {
            openFiles.insert(localPath);
            updates[localPath] = move(openParams.textDocument->text);
        }
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidCloseTextDocumentParams &closeParams,
                                            UnorderedMap<string, string> &updates) {
    string_view uri = closeParams.textDocument->uri;
    if (absl::StartsWith(uri, rootUri)) {
        string localPath = remoteName2Local(uri);
        if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns)) {
            auto it = openFiles.find(localPath);
            if (it != openFiles.end()) {
                openFiles.erase(it);
            }
            // Use contents of file on disk.
            updates[localPath] = readFile(localPath, *opts.fs);
        }
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const WatchmanQueryResponse &queryResponse,
                                            UnorderedMap<string, string> &updates) {
    for (auto file : queryResponse.files) {
        string localPath = absl::StrCat(rootPath, "/", file);
        if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns) &&
            openFiles.find(localPath) == openFiles.end()) {
            updates[localPath] = readFile(localPath, *opts.fs);
        }
    }
}

LSPResult LSPLoop::commitSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs, UnorderedMap<string, string> &updates) {
    if (updates.size() > 0) {
        vector<shared_ptr<core::File>> files;
        files.reserve(updates.size());
        for (auto &update : updates) {
            files.push_back(
                make_shared<core::File>(string(update.first), move(update.second), core::File::Type::Normal));
        }
        return pushDiagnostics(tryFastPath(move(gs), files));
    } else {
        return LSPResult{move(gs), {}};
    }
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidChangeTextDocumentParams &changeParams) {
    UnorderedMap<string, string> updates;
    preprocessSorbetWorkspaceEdit(changeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidOpenTextDocumentParams &openParams) {
    UnorderedMap<string, string> updates;
    preprocessSorbetWorkspaceEdit(openParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidCloseTextDocumentParams &closeParams) {
    UnorderedMap<string, string> updates;
    preprocessSorbetWorkspaceEdit(closeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const WatchmanQueryResponse &queryResponse) {
    UnorderedMap<string, string> updates;
    preprocessSorbetWorkspaceEdit(queryResponse, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                              vector<unique_ptr<SorbetWorkspaceEdit>> &edits) {
    // path => new file contents
    UnorderedMap<string, string> updates;
    for (auto &edit : edits) {
        switch (edit->type) {
            case SorbetWorkspaceEditType::EditorOpen: {
                preprocessSorbetWorkspaceEdit(*get<unique_ptr<DidOpenTextDocumentParams>>(edit->contents), updates);
                break;
            }
            case SorbetWorkspaceEditType::EditorChange: {
                preprocessSorbetWorkspaceEdit(*get<unique_ptr<DidChangeTextDocumentParams>>(edit->contents), updates);
                break;
            }
            case SorbetWorkspaceEditType::EditorClose: {
                preprocessSorbetWorkspaceEdit(*get<unique_ptr<DidCloseTextDocumentParams>>(edit->contents), updates);
                break;
            }
            case SorbetWorkspaceEditType::FileSystem: {
                preprocessSorbetWorkspaceEdit(*get<unique_ptr<WatchmanQueryResponse>>(edit->contents), updates);
                break;
            }
        }
    }
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

} // namespace sorbet::realmain::lsp