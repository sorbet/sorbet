#include "main/lsp/LSPPreprocessor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/lsp.h"
#include "main/pipeline/pipeline.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
bool sanityCheckUpdate(const core::GlobalState &gs, const LSPFileUpdates &updates) {
    UnorderedSet<string> encounteredFiles;
    ENFORCE(updates.updatedFiles.size() == updates.updatedFileHashes.size());
    ENFORCE(updates.updatedFiles.size() == updates.updatedFileIndexes.size());
    for (int i = 0; i < updates.updatedFiles.size(); i++) {
        const auto &f = updates.updatedFiles[i];
        // const auto &h = updates.updatedFileHashes[i];
        const auto &ast = updates.updatedFileIndexes[i];
        ENFORCE(f->path() == ast.file.data(gs).path());
        ENFORCE(!encounteredFiles.contains(f->path()));
        encounteredFiles.insert(string(f->path()));
    }
    ENFORCE(updates.canTakeFastPath || updates.updatedGS.has_value());
    if (updates.hasNewFiles) {
        ENFORCE(!updates.canTakeFastPath);
    }
    return true;
}

void cancelTimer(unique_ptr<Timer> &timer) {
    // Protect against nullptrs.
    if (timer) {
        timer->cancel();
    }
}

} // namespace

LSPPreprocessor::LSPPreprocessor(unique_ptr<core::GlobalState> initialGS, const shared_ptr<LSPConfiguration> &config,
                                 u4 initialVersion)
    : ttgs(TimeTravelingGlobalState(config, move(initialGS), initialVersion)), config(config),
      owner(this_thread::get_id()), nextVersion(initialVersion + 1) {}

void LSPPreprocessor::mergeFileChanges(absl::Mutex &mtx, QueueState &state) {
    mtx.AssertHeld();
    auto &logger = config->logger;
    // mergeFileChanges is the most expensive operation this thread performs while holding the mutex lock.
    Timer timeit(logger, "lsp.mergeFileChanges");
    u4 earliestActiveEditVersion = nextVersion;
    auto &pendingRequests = state.pendingRequests;
    const int originalSize = pendingRequests.size();
    int requestsMergedCounter = 0;

    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto &msg = **it;
        it++;
        if (!msg.isNotification() || msg.method() != LSPMethod::SorbetWorkspaceEdit) {
            continue;
        }
        auto &msgParams = get<unique_ptr<SorbetWorkspaceEditParams>>(msg.asNotification().params);
        // See which newer requests we can enqueue. We want to merge them *backwards* into msgParams.
        earliestActiveEditVersion = ttgs.minVersion(earliestActiveEditVersion, msgParams->updates.versionStart);
        while (it != pendingRequests.end()) {
            auto &mergeMsg = **it;
            const bool canMerge = mergeMsg.isNotification() && mergeMsg.method() == LSPMethod::SorbetWorkspaceEdit;
            if (!canMerge) {
                if (mergeMsg.isDelayable()) {
                    ++it;
                    continue;
                } else {
                    // Stop merging if the pointed-to message failed to merge AND is not a delayable message.
                    break;
                }
            }

            // Merge updates and tracers, and cancel its timer to avoid a distorted latency metric.
            auto &mergeableParams = get<unique_ptr<SorbetWorkspaceEditParams>>(mergeMsg.asNotification().params);
            mergeEdits(msgParams->updates, mergeableParams->updates);
            cancelTimer(msg.timer);
            msg.startTracers.insert(msg.startTracers.end(), mergeMsg.startTracers.begin(), mergeMsg.startTracers.end());
            // Delete the update we just merged and move on to next item.
            it = pendingRequests.erase(it);
            requestsMergedCounter++;
        }
    }
    ENFORCE(pendingRequests.size() + requestsMergedCounter == originalSize);

    // Check if we should cancel the slow path.
    const auto &gs = ttgs.getGlobalState();
    const auto runningSlowPath = gs.getRunningSlowPath();
    // Only try to cancel if the typechecking thread is running the slow path.
    if (runningSlowPath.has_value()) {
        const auto &[committed, end] = runningSlowPath.value();
        earliestActiveEditVersion = ttgs.minVersion(committed, earliestActiveEditVersion);

        // Avoid canceling if the currently-running slow path has already been canceled.
        if (!gs.wasTypecheckingCanceled()) {
            for (auto it = pendingRequests.begin(); it != pendingRequests.end(); it++) {
                const auto &msg = *it;
                if (msg->isNotification() && msg->method() == LSPMethod::SorbetWorkspaceEdit) {
                    Timer timeit(logger, "tryCancelSlowPath");
                    auto &params = get<unique_ptr<SorbetWorkspaceEditParams>>(msg->asNotification().params);
                    auto combinedUpdates = ttgs.getCombinedUpdates(committed + 1, params->updates.versionEnd);
                    // Cancel if combined updates end up taking the fast path, or if the new updates will just take the
                    // slow path a second time when the current slow path finishes.
                    if ((combinedUpdates.canTakeFastPath || !params->updates.canTakeFastPath) &&
                        gs.tryCancelSlowPath(params->updates.versionEnd)) {
                        if (combinedUpdates.canTakeFastPath) {
                            logger->debug(
                                "[Preprocessor] Canceling typechecking, as edits {} thru {} can take fast path.",
                                combinedUpdates.versionStart, combinedUpdates.versionEnd);
                        } else {
                            logger->debug(
                                "[Preprocessor] Canceling typechecking, as new edits {} thru {} will just take "
                                "the slow path again.",
                                params->updates.versionStart, params->updates.versionEnd);
                            combinedUpdates.updatedGS = getTypecheckingGS();
                        }
                        params->updates = move(combinedUpdates);
                    }
                    break;
                } else if (!msg->isDelayable()) {
                    // Message is not delayable, and is not an edit. Can't cancel the slow path.
                    break;
                }
            }
        }
    }

    // Prune history for all messages no longer in queue and no longer being typechecked.
    ttgs.pruneBefore(earliestActiveEditVersion);
}

void cancelRequest(std::deque<std::unique_ptr<LSPMessage>> &pendingRequests, const CancelParams &cancelParams) {
    for (auto &current : pendingRequests) {
        if (current->isRequest()) {
            auto &request = current->asRequest();
            if (request.id == cancelParams.id) {
                // We didn't start processing it yet -- great! Cancel it and return.
                current->canceled = true;
                // Don't report a latency metric for canceled requests.
                cancelTimer(current->timer);
                return;
            }
        }
    }
    // Else... it's too late; we have either already processed it, or are currently processing it. Swallow cancellation
    // and ignore.
}

unique_ptr<core::GlobalState> LSPPreprocessor::getTypecheckingGS() const {
    return ttgs.getGlobalState().deepCopy();
}

unique_ptr<LSPMessage> LSPPreprocessor::makeAndCommitWorkspaceEdit(unique_ptr<SorbetWorkspaceEditParams> params,
                                                                   unique_ptr<LSPMessage> oldMsg) {
    ttgs.commitEdits(params->updates);
    if (!params->updates.canTakeFastPath) {
        params->updates.updatedGS = getTypecheckingGS();
    }
    auto newMsg =
        make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWorkspaceEdit, move(params)));
    newMsg->timer = move(oldMsg->timer);
    newMsg->startTracers = move(oldMsg->startTracers);
    return newMsg;
}

bool LSPPreprocessor::ensureInitialized(LSPMethod method, const LSPMessage &msg) const {
    if (config->isInitialized() || method == LSPMethod::Initialize || method == LSPMethod::Initialized ||
        method == LSPMethod::Exit || method == LSPMethod::Shutdown || method == LSPMethod::SorbetError) {
        return true;
    }
    config->logger->error("Serving request before got an Initialize & Initialized handshake from IDE");
    vector<unique_ptr<LSPMessage>> responses;
    if (!msg.isNotification()) {
        auto id = msg.id().value_or(0);
        auto response = make_unique<ResponseMessage>("2.0", id, msg.method());
        response->error = make_unique<ResponseError>((int)LSPErrorCodes::ServerNotInitialized,
                                                     "IDE did not initialize Sorbet correctly. No requests should "
                                                     "be made before Initialize & Initialized have been completed");
        config->output->write(move(response));
    }
    return false;
}

void LSPPreprocessor::preprocessAndEnqueue(QueueState &state, unique_ptr<LSPMessage> msg, absl::Mutex &stateMtx) {
    ENFORCE(owner == this_thread::get_id());
    if (msg->isResponse()) {
        return;
    }

    const LSPMethod method = msg->method();
    if (!ensureInitialized(method, *msg)) {
        // msg is invalid. Error sent to client.
        return;
    }

    auto &logger = config->logger;
    bool shouldEnqueue = false;
    bool shouldMerge = false;
    // Ensure TTGS has file contents from previous edit.
    ttgs.travel(nextVersion - 1);
    switch (method) {
        case LSPMethod::$CancelRequest: {
            absl::MutexLock lock(&stateMtx);
            cancelRequest(state.pendingRequests, *get<unique_ptr<CancelParams>>(msg->asNotification().params));
            // A canceled request can be moved around, so we may be able to merge more file changes.
            mergeFileChanges(stateMtx, state);
            break;
        }
        case LSPMethod::PAUSE: {
            absl::MutexLock lock(&stateMtx);
            ENFORCE(!state.paused);
            logger->error("Pausing");
            state.paused = true;
            break;
        }
        case LSPMethod::RESUME: {
            absl::MutexLock lock(&stateMtx);
            logger->error("Resuming");
            ENFORCE(state.paused);
            state.paused = false;
            break;
        }
        case LSPMethod::Exit: {
            absl::MutexLock lock(&stateMtx);
            // Don't override previous error code if already terminated.
            if (!state.terminate) {
                state.terminate = true;
                state.errorCode = 0;
            }
            state.pendingRequests.push_back(move(msg));
            break;
        }
        case LSPMethod::Initialize: {
            // Update configuration object. Needed to intelligently process edits.
            const auto &params = get<unique_ptr<InitializeParams>>(msg->asRequest().params);
            config->setClientConfig(make_shared<LSPClientConfiguration>(*params));
            shouldEnqueue = true;
            break;
        }
        case LSPMethod::Initialized: {
            InitializedParams &params = *get<unique_ptr<InitializedParams>>(msg->asNotification().params);
            {
                Timer timeit(logger, "initial_index");
                ShowOperation op(*config, "Indexing", "Indexing files...");
                params.updates.updatedFileIndexes = ttgs.indexFromFileSystem();
                params.updates.updatedFileHashes = ttgs.getGlobalStateHashes();
            }
            config->markInitialized();
            params.updates.canTakeFastPath = false;
            params.updates.updatedGS = getTypecheckingGS();
            shouldEnqueue = true;
            break;
        }
        /* For file update events, convert to a SorbetWorkspaceEdit and commit the changes to GlobalState. */
        case LSPMethod::TextDocumentDidOpen: {
            auto &params = get<unique_ptr<DidOpenTextDocumentParams>>(msg->asNotification().params);
            // Ignore files not in workspace.
            if (config->isUriInWorkspace(params->textDocument->uri)) {
                openFiles.insert(config->remoteName2Local(params->textDocument->uri));
                auto newParams = make_unique<SorbetWorkspaceEditParams>();
                canonicalizeEdits(nextVersion++, move(params), newParams->updates);
                msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
                shouldEnqueue = shouldMerge = true;
            }
            break;
        }
        case LSPMethod::TextDocumentDidClose: {
            auto &params = get<unique_ptr<DidCloseTextDocumentParams>>(msg->asNotification().params);
            if (config->isUriInWorkspace(params->textDocument->uri)) {
                openFiles.erase(config->remoteName2Local(params->textDocument->uri));
                auto newParams = make_unique<SorbetWorkspaceEditParams>();
                canonicalizeEdits(nextVersion++, move(params), newParams->updates);
                msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
                shouldEnqueue = shouldMerge = true;
            }
            break;
        }
        case LSPMethod::TextDocumentDidChange: {
            auto &params = get<unique_ptr<DidChangeTextDocumentParams>>(msg->asNotification().params);
            if (config->isUriInWorkspace(params->textDocument->uri)) {
                auto newParams = make_unique<SorbetWorkspaceEditParams>();
                canonicalizeEdits(nextVersion++, move(params), newParams->updates);
                msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
                shouldEnqueue = shouldMerge = true;
            }
            break;
        }
        case LSPMethod::SorbetWatchmanFileChange: {
            auto &params = get<unique_ptr<WatchmanQueryResponse>>(msg->asNotification().params);
            auto newParams = make_unique<SorbetWorkspaceEditParams>();
            canonicalizeEdits(nextVersion++, move(params), newParams->updates);
            if (newParams->updates.updatedFiles.empty()) {
                // No need to commit; these file system updates are ignored.
                // Reclaim edit version, as we didn't actually use this one.
                nextVersion--;
                return;
            }
            msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
            shouldEnqueue = shouldMerge = true;
            break;
        }
        default: {
            // No need to merge; this isn't a file edit.
            shouldEnqueue = true;
            break;
        }
    }

    if (shouldEnqueue || shouldMerge) {
        absl::MutexLock lock(&stateMtx);
        if (shouldEnqueue) {
            state.pendingRequests.push_back(move(msg));
        }
        if (shouldMerge) {
            mergeFileChanges(stateMtx, state);
        }
    }
}

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

string_view getFileContents(LSPFileUpdates &updates, const core::GlobalState &initialGS, string_view path) {
    // Get last file in array matching path. There may be duplicates (which will be culled before committing).
    const auto &updatedFiles = updates.updatedFiles;
    for (auto it = updatedFiles.rbegin(); it != updatedFiles.rend(); it++) {
        if ((*it)->path() == path) {
            return (*it)->source();
        }
    }

    auto currentFileRef = initialGS.findFileByPath(path);
    if (currentFileRef.exists()) {
        return currentFileRef.data(initialGS).source();
    } else {
        return "";
    }
}

void LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<DidChangeTextDocumentParams> changeParams,
                                        LSPFileUpdates &updates) const {
    updates.versionStart = v;
    updates.versionEnd = v;
    string_view uri = changeParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (config->isFileIgnored(localPath)) {
            return;
        }
        string fileContents;
        for (auto &change : changeParams->contentChanges) {
            if (change->range) {
                fileContents = string(getFileContents(updates, ttgs.getGlobalState(), localPath));
                auto &range = *change->range;
                // incremental update
                core::Loc::Detail start, end;
                start.line = range->start->line + 1;
                start.column = range->start->character + 1;
                end.line = range->end->line + 1;
                end.column = range->end->character + 1;
                core::File old(string(localPath), string(fileContents), core::File::Type::Normal);
                // These offsets are non-nullopt assuming the input range is a valid range.
                auto startOffset = core::Loc::pos2Offset(old, start).value();
                auto endOffset = core::Loc::pos2Offset(old, end).value();
                fileContents = fileContents.replace(startOffset, endOffset - startOffset, change->text);
            } else {
                // replace
                fileContents = move(change->text);
            }
        }
        updates.updatedFiles.push_back(
            make_shared<core::File>(move(localPath), move(fileContents), core::File::Type::Normal));
    }
}

void LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<DidOpenTextDocumentParams> openParams,
                                        LSPFileUpdates &updates) const {
    updates.versionStart = v;
    updates.versionEnd = v;
    string_view uri = openParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            updates.updatedFiles.push_back(make_shared<core::File>(
                move(localPath), move(openParams->textDocument->text), core::File::Type::Normal));
        }
    }
}

void LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<DidCloseTextDocumentParams> closeParams,
                                        LSPFileUpdates &updates) const {
    updates.versionStart = v;
    updates.versionEnd = v;
    string_view uri = closeParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            // Use contents of file on disk.
            updates.updatedFiles.push_back(make_shared<core::File>(
                move(localPath), readFile(localPath, *config->opts.fs), core::File::Type::Normal));
        }
    }
}

void LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<WatchmanQueryResponse> queryResponse,
                                        LSPFileUpdates &updates) const {
    updates.versionStart = v;
    updates.versionEnd = v;
    for (auto file : queryResponse->files) {
        // Don't append rootPath if it is empty.
        string localPath = config->rootPath.size() > 0 ? absl::StrCat(config->rootPath, "/", file) : file;
        // Editor contents supercede file system updates.
        if (!config->isFileIgnored(localPath) && !openFiles.contains(localPath)) {
            updates.updatedFiles.push_back(make_shared<core::File>(
                move(localPath), readFile(localPath, *config->opts.fs), core::File::Type::Normal));
        }
    }
}

void LSPPreprocessor::mergeEdits(LSPFileUpdates &to, LSPFileUpdates &from) {
    ENFORCE(sanityCheckUpdate(ttgs.getGlobalState(), to));
    ENFORCE(sanityCheckUpdate(ttgs.getGlobalState(), from));

    // fromId must happen *after* toId.
    ENFORCE(ttgs.comesBefore(to.versionEnd, from.versionEnd));
    // 'from' has newer updates, so merge into from and then move into to.
    UnorderedSet<int> encounteredFiles;
    for (auto &index : from.updatedFileIndexes) {
        encounteredFiles.insert(index.file.id());
    }
    int i = -1;
    for (auto &index : to.updatedFileIndexes) {
        i++;
        if (!encounteredFiles.contains(index.file.id())) {
            encounteredFiles.insert(index.file.id());
            from.updatedFileIndexes.push_back(move(index));
            from.updatedFiles.push_back(move(to.updatedFiles[i]));
            from.updatedFileHashes.push_back(move(to.updatedFileHashes[i]));
        }
    }
    to.updatedFiles = move(from.updatedFiles);
    to.updatedFileIndexes = move(from.updatedFileIndexes);
    to.updatedFileHashes = move(from.updatedFileHashes);
    to.hasNewFiles = to.hasNewFiles || from.hasNewFiles;
    to.canTakeFastPath = ttgs.canTakeFastPath(to.versionStart - 1, to);
    // `to` now includes the contents of `from`.
    to.versionEnd = from.versionEnd;
    // No need to update versionStart, as to comes before from.
    ENFORCE(ttgs.comesBefore(to.versionStart, from.versionStart));
    if (to.canTakeFastPath) {
        prodCategoryCounterInc("lsp.merge_edits", "fast_path");
        to.updatedGS = nullopt;
    } else if (from.updatedGS.has_value()) {
        // `from` has all file updates from `to` and `from` so its GS can be re-used if specified.
        prodCategoryCounterInc("lsp.merge_edits", "slow_path_reuse_gs");
        to.updatedGS = move(from.updatedGS.value());
    } else {
        // Roll forward again so initial GS has changes from this update prior to copying.
        prodCategoryCounterInc("lsp.merge_edits", "slow_path_new_gs");
        ttgs.travel(from.versionEnd);
        to.updatedGS = getTypecheckingGS();
    }
    ENFORCE(sanityCheckUpdate(ttgs.getGlobalState(), to));
}

} // namespace sorbet::realmain::lsp
