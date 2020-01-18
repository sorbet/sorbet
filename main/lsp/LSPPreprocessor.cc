#include "main/lsp/LSPPreprocessor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
void cancelRequest(std::deque<std::unique_ptr<LSPMessage>> &pendingRequests, const CancelParams &cancelParams) {
    for (auto &current : pendingRequests) {
        if (current->isRequest()) {
            auto &request = current->asRequest();
            if (request.id == cancelParams.id) {
                // We didn't start processing it yet -- great! Cancel it and return.
                current->cancel();
                return;
            }
        }
    }
    // Else... it's too late; we have either already processed it, or are currently processing it. Swallow cancellation
    // and ignore.
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
} // namespace

LSPPreprocessor::LSPPreprocessor(shared_ptr<LSPConfiguration> config, u4 initialVersion)
    : config(move(config)), owner(this_thread::get_id()), nextVersion(initialVersion + 1) {}

string_view LSPPreprocessor::getFileContents(string_view path) const {
    auto it = openFiles.find(path);
    if (it == openFiles.end()) {
        ENFORCE(false, "Editor sent a change request without a matching open request.");
        return string_view();
    }
    return it->second->source();
}

void LSPPreprocessor::mergeFileChanges(absl::Mutex &mtx, QueueState &state) {
    mtx.AssertHeld();
    auto &logger = config->logger;
    // mergeFileChanges is the most expensive operation this thread performs while holding the mutex lock.
    Timer timeit(logger, "lsp.mergeFileChanges");
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
            msgParams->merge(*mergeableParams);
            msg.startTracers.insert(msg.startTracers.end(), mergeMsg.startTracers.begin(), mergeMsg.startTracers.end());
            mergeMsg.cancel();
            // Delete the update we just merged and move on to next item.
            it = pendingRequests.erase(it);
            requestsMergedCounter++;
        }
    }
    ENFORCE(pendingRequests.size() + requestsMergedCounter == originalSize);
}

unique_ptr<LSPMessage> LSPPreprocessor::makeAndCommitWorkspaceEdit(unique_ptr<SorbetWorkspaceEditParams> params,
                                                                   unique_ptr<LSPMessage> oldMsg) {
    auto newMsg =
        make_unique<LSPMessage>(make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWorkspaceEdit, move(params)));
    newMsg->timer = move(oldMsg->timer);
    newMsg->startTracers = move(oldMsg->startTracers);
    return newMsg;
}

bool LSPPreprocessor::ensureInitialized(LSPMethod method, const LSPMessage &msg) const {
    if (config->isInitialized() || method == LSPMethod::Initialize || method == LSPMethod::Initialized ||
        method == LSPMethod::Exit || method == LSPMethod::Shutdown || method == LSPMethod::SorbetError ||
        method == LSPMethod::SorbetFence) {
        return true;
    }
    config->logger->error("Serving request before got an Initialize & Initialized handshake from IDE");
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
            config->markInitialized();
            shouldEnqueue = true;
            break;
        }
        /* For file update events, convert to a SorbetWorkspaceEdit and commit the changes to GlobalState. */
        case LSPMethod::TextDocumentDidOpen: {
            auto &params = get<unique_ptr<DidOpenTextDocumentParams>>(msg->asNotification().params);
            // Ignore files not in workspace.
            if (config->isUriInWorkspace(params->textDocument->uri)) {
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
                shouldEnqueue = shouldMerge = true;
            }
            break;
        }
        case LSPMethod::TextDocumentDidClose: {
            auto &params = get<unique_ptr<DidCloseTextDocumentParams>>(msg->asNotification().params);
            if (config->isUriInWorkspace(params->textDocument->uri)) {
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
                shouldEnqueue = shouldMerge = true;
            }
            break;
        }
        case LSPMethod::TextDocumentDidChange: {
            auto &params = get<unique_ptr<DidChangeTextDocumentParams>>(msg->asNotification().params);
            if (config->isUriInWorkspace(params->textDocument->uri)) {
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                msg = makeAndCommitWorkspaceEdit(move(newParams), move(msg));
                shouldEnqueue = shouldMerge = true;
            }
            break;
        }
        case LSPMethod::SorbetWatchmanFileChange: {
            auto &params = get<unique_ptr<WatchmanQueryResponse>>(msg->asNotification().params);
            auto newParams = canonicalizeEdits(nextVersion++, move(params));
            if (newParams->updates.empty()) {
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

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<DidChangeTextDocumentParams> changeParams) {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    edit->sorbetCancellationExpected = changeParams->sorbetCancellationExpected.value_or(false);
    string_view uri = changeParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            string fileContents = changeParams->getSource(getFileContents(localPath));
            auto file = make_shared<core::File>(move(localPath), move(fileContents), core::File::Type::Normal, v);
            edit->updates.push_back(file);
            openFiles[localPath] = move(file);
        }
    }
    return edit;
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<DidOpenTextDocumentParams> openParams) {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    string_view uri = openParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            auto file = make_shared<core::File>(move(localPath), move(openParams->textDocument->text),
                                                core::File::Type::Normal, v);
            edit->updates.push_back(file);
            openFiles[localPath] = move(file);
        }
    }
    return edit;
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<DidCloseTextDocumentParams> closeParams) {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    string_view uri = closeParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            openFiles.erase(localPath);
            // Use contents of file on disk.
            edit->updates.push_back(make_shared<core::File>(move(localPath), readFile(localPath, *config->opts.fs),
                                                            core::File::Type::Normal, v));
        }
    }
    return edit;
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(u4 v, unique_ptr<WatchmanQueryResponse> queryResponse) const {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    for (auto file : queryResponse->files) {
        // Don't append rootPath if it is empty.
        string localPath = !config->rootPath.empty() ? absl::StrCat(config->rootPath, "/", file) : file;
        // Editor contents supercede file system updates.
        if (!config->isFileIgnored(localPath) && !openFiles.contains(localPath)) {
            edit->updates.push_back(make_shared<core::File>(move(localPath), readFile(localPath, *config->opts.fs),
                                                            core::File::Type::Normal, v));
        }
    }
    return edit;
}

} // namespace sorbet::realmain::lsp
