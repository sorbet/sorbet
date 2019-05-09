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

string getFileContents(UnorderedMap<string, pair<string, bool>> &updates, LSPState &state, const string &path)
    EXCLUSIVE_LOCKS_REQUIRED(state.mtx) {
    if (updates.find(path) != updates.end()) {
        return updates[path].first;
    }

    auto contents = state.getFileContents(state.findFileByPath(path));
    if (contents) {
        return *contents;
    } else {
        return "";
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidChangeTextDocumentParams &changeParams,
                                            UnorderedMap<string, pair<string, bool>> &updates) {
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
        updates[localPath] = make_pair(fileContents, true);
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidOpenTextDocumentParams &openParams,
                                            UnorderedMap<string, pair<string, bool>> &updates) {
    string_view uri = openParams.textDocument->uri;
    if (absl::StartsWith(uri, rootUri)) {
        string localPath = remoteName2Local(uri);
        if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns)) {
            updates[localPath] = make_pair(move(openParams.textDocument->text), true);
        }
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const DidCloseTextDocumentParams &closeParams,
                                            UnorderedMap<string, pair<string, bool>> &updates) {
    string_view uri = closeParams.textDocument->uri;
    if (absl::StartsWith(uri, rootUri)) {
        string localPath = remoteName2Local(uri);
        if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns)) {
            // Use contents of file on disk.
            updates[localPath] = make_pair(readFile(localPath, *opts.fs), false);
        }
    }
}

bool isFileOpen(LSPState &state, UnorderedMap<string, pair<string, bool>> &updates, string_view path)
    EXCLUSIVE_LOCKS_REQUIRED(state.mtx) {
    return state.isFileOpen(path) || (updates.find(string(path)) != updates.end() && updates[string(path)].second);
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const WatchmanQueryResponse &queryResponse,
                                            UnorderedMap<string, pair<string, bool>> &updates) {
    for (auto file : queryResponse.files) {
        string localPath = absl::StrCat(rootPath, "/", file);
        if (!FileOps::isFileIgnored(rootPath, localPath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns) &&
            !isFileOpen(state, updates, localPath)) {
            updates[localPath] = make_pair(readFile(localPath, *opts.fs), false);
        }
    }
}

LSPResult LSPLoop::commitSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                              UnorderedMap<string, pair<string, bool>> &updates) {
    if (updates.size() > 0) {
        ShowOperation op(*this, "typechecking", "Sorbet: Typechecking...");
        return pushDiagnostics(state.runTypechecking(move(gs), updates));
    } else {
        return LSPResult{move(gs), {}};
    }
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidChangeTextDocumentParams &changeParams) {
    UnorderedMap<string, pair<string, bool>> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(changeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidOpenTextDocumentParams &openParams) {
    UnorderedMap<string, pair<string, bool>> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(openParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const DidCloseTextDocumentParams &closeParams) {
    UnorderedMap<string, pair<string, bool>> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(closeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPResult LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                             const WatchmanQueryResponse &queryResponse) {
    UnorderedMap<string, pair<string, bool>> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdit(queryResponse, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

void LSPLoop::preprocessSorbetWorkspaceEdits(const vector<unique_ptr<SorbetWorkspaceEdit>> &edits,
                                             UnorderedMap<string, pair<string, bool>> &updates) {
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
}

LSPResult LSPLoop::handleSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                              const vector<unique_ptr<SorbetWorkspaceEdit>> &edits) {
    // path => new file contents
    UnorderedMap<string, pair<string, bool>> updates;
    absl::MutexLock lock(&state.mtx);
    preprocessSorbetWorkspaceEdits(edits, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

} // namespace sorbet::realmain::lsp