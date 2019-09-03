#include "absl/strings/match.h"
#include "common/FileOps.h"
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

string_view LSPLoop::getFileContents(UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> &updates,
                                     const core::GlobalState &initialGS, string_view path) {
    if (updates.find(path) != updates.end()) {
        return updates[path].contents;
    }

    auto currentFileRef = initialGS.findFileByPath(path);
    if (currentFileRef.exists()) {
        return currentFileRef.data(initialGS).source();
    } else {
        return "";
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const LSPConfiguration &config, const core::GlobalState &initialGS,
                                            const DidChangeTextDocumentParams &changeParams,
                                            UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> &updates) {
    string_view uri = changeParams.textDocument->uri;
    if (absl::StartsWith(uri, config.rootUri)) {
        string localPath = config.remoteName2Local(uri);
        if (config.isFileIgnored(localPath)) {
            return;
        }
        string fileContents = string(getFileContents(updates, initialGS, localPath));
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
        updates[localPath] = {fileContents, /* newlyOpened */ false, /* newlyClosed */ false};
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const LSPConfiguration &config, const DidOpenTextDocumentParams &openParams,
                                            UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> &updates) {
    string_view uri = openParams.textDocument->uri;
    if (absl::StartsWith(uri, config.rootUri)) {
        string localPath = config.remoteName2Local(uri);
        if (!config.isFileIgnored(localPath)) {
            updates[localPath] = {move(openParams.textDocument->text), /* newlyOpened */ true, /* newlyClosed */ false};
        }
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const LSPConfiguration &config,
                                            const DidCloseTextDocumentParams &closeParams,
                                            UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> &updates) {
    string_view uri = closeParams.textDocument->uri;
    if (absl::StartsWith(uri, config.rootUri)) {
        string localPath = config.remoteName2Local(uri);
        if (!config.isFileIgnored(localPath)) {
            // Use contents of file on disk.
            updates[localPath] = {readFile(localPath, *config.opts.fs), /* newlyOpened */ false,
                                  /* newlyClosed */ true};
        }
    }
}

void LSPLoop::preprocessSorbetWorkspaceEdit(const LSPConfiguration &config, const UnorderedSet<std::string> openFiles,
                                            const WatchmanQueryResponse &queryResponse,
                                            UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> &updates) {
    for (auto file : queryResponse.files) {
        string localPath = absl::StrCat(config.rootPath, "/", file);
        if (!config.isFileIgnored(localPath)) {
            auto it = updates.find(localPath);
            bool isFileOpenInEditor = openFiles.contains(localPath);
            if (it != updates.end()) {
                // File is newly opened (true), opened previously but now closed (false), or opened previously and still
                // open (true).
                isFileOpenInEditor = it->second.newlyOpened || (isFileOpenInEditor && !it->second.newlyClosed);
            }
            // Editor contents supercede file system updates.
            if (!isFileOpenInEditor) {
                // File may have been closed and then updated on disk, so make sure we preserve the 'closed' flag.
                updates[localPath] = {readFile(localPath, *config.opts.fs), /* newlyOpened */ false,
                                      /* newlyClosed */ it != updates.end() ? it->second.newlyClosed : false};
            }
        }
    }
}

LSPLoop::TypecheckRun
LSPLoop::commitSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                    UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> &updates) const {
    if (!updates.empty()) {
        FileUpdates fileUpdates;
        fileUpdates.updatedFiles.reserve(updates.size());
        for (auto &update : updates) {
            auto file =
                make_shared<core::File>(string(update.first), move(update.second.contents), core::File::Type::Normal);
            if (update.second.newlyClosed) {
                fileUpdates.closedFiles.push_back(string(file->path()));
            }
            if (update.second.newlyOpened) {
                fileUpdates.openedFiles.push_back(string(file->path()));
            }
            fileUpdates.updatedFiles.push_back(move(file));
        }
        return runTypechecking(move(gs), move(fileUpdates));
    } else {
        return TypecheckRun{{}, {}, move(gs), {}, true};
    }
}

LSPLoop::TypecheckRun LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                                         const DidChangeTextDocumentParams &changeParams) const {
    UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> updates;
    preprocessSorbetWorkspaceEdit(config, *initialGS, changeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPLoop::TypecheckRun LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                                         const DidOpenTextDocumentParams &openParams) const {
    UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> updates;
    preprocessSorbetWorkspaceEdit(config, openParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPLoop::TypecheckRun LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                                         const DidCloseTextDocumentParams &closeParams) const {
    UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> updates;
    preprocessSorbetWorkspaceEdit(config, closeParams, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPLoop::TypecheckRun LSPLoop::handleSorbetWorkspaceEdit(unique_ptr<core::GlobalState> gs,
                                                         const WatchmanQueryResponse &queryResponse) const {
    UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> updates;
    preprocessSorbetWorkspaceEdit(config, openFiles, queryResponse, updates);
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

LSPLoop::TypecheckRun LSPLoop::handleSorbetWorkspaceEdits(unique_ptr<core::GlobalState> gs,
                                                          vector<unique_ptr<SorbetWorkspaceEdit>> &edits) const {
    // path => new file contents
    UnorderedMap<string, LSPLoop::SorbetWorkspaceFileUpdate> updates;
    for (auto &edit : edits) {
        switch (edit->type) {
            case SorbetWorkspaceEditType::EditorOpen: {
                preprocessSorbetWorkspaceEdit(config, *get<unique_ptr<DidOpenTextDocumentParams>>(edit->contents),
                                              updates);
                break;
            }
            case SorbetWorkspaceEditType::EditorChange: {
                preprocessSorbetWorkspaceEdit(config, *initialGS,
                                              *get<unique_ptr<DidChangeTextDocumentParams>>(edit->contents), updates);
                break;
            }
            case SorbetWorkspaceEditType::EditorClose: {
                preprocessSorbetWorkspaceEdit(config, *get<unique_ptr<DidCloseTextDocumentParams>>(edit->contents),
                                              updates);
                break;
            }
            case SorbetWorkspaceEditType::FileSystem: {
                preprocessSorbetWorkspaceEdit(config, openFiles,
                                              *get<unique_ptr<WatchmanQueryResponse>>(edit->contents), updates);
                break;
            }
        }
    }
    return commitSorbetWorkspaceEdits(move(gs), updates);
}

} // namespace sorbet::realmain::lsp
