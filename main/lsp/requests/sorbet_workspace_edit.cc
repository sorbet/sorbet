#include "absl/strings/match.h"
#include "common/FileSystem.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"
#include "main/options/options.h"

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

string getFileContents(UnorderedMap<string, string> &updates, LSPState &state, const string &path)
    EXCLUSIVE_LOCKS_REQUIRED(state.mtx) {
    if (updates.find(path) != updates.end()) {
        return updates[path];
    }

    auto contents = state.getFileContents(state.findFileByPath(path));
    if (contents) {
        return *contents;
    } else {
        return "";
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidChangeTextDocumentParams &changeParams,
                                            UnorderedMap<string, string> &updates) {
    string_view uri = changeParams.textDocument->uri;
    if (absl::StartsWith(uri, rootUri)) {
        string localPath = remoteName2Local(uri);
        if (FileOps::isFileIgnored(rootPath, localPath, state.opts.absoluteIgnorePatterns,
                                   state.opts.relativeIgnorePatterns)) {
            return;
        }
        string fileContents = getFileContents(updates, state, localPath);
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
            state.openFile(localPath);
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
            state.closeFile(localPath);
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
            !state.isFileOpen(localPath)) {
            updates[localPath] = readFile(localPath, *opts.fs);
        }
    }
}

LSPResult LSPLoop::commitSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs, UnorderedMap<string, string> &updates)
    EXCLUSIVE_LOCKS_REQUIRED(state.mtx) {
    if (updates.size() > 0) {
        vector<shared_ptr<core::File>> files;
        files.reserve(updates.size());
        for (auto &update : updates) {
            files.push_back(
                make_shared<core::File>(string(update.first), move(update.second), core::File::Type::Normal));
        }
        return pushDiagnostics(state.tryFastPath(move(gs), files));
    } else {
        return LSPResult{move(gs), {}};
    }
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidChangeTextDocumentParams &changeParams) {
    UnorderedMap<string, string> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(changeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidOpenTextDocumentParams &openParams) {
    UnorderedMap<string, string> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(openParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidCloseTextDocumentParams &closeParams) {
    UnorderedMap<string, string> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(closeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const WatchmanQueryResponse &queryResponse) {
    UnorderedMap<string, string> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(queryResponse, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                              vector<unique_ptr<SorbetWorkspaceEdit>> &edits) {
    // path => new file contents
    UnorderedMap<string, string> updates;
    absl::MutexLock lock(&state.mtx);
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