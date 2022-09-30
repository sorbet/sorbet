#include "main/lsp/LSPPreprocessor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "main/lsp/notifications/notifications.h"
#include "main/lsp/requests/requests.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
string readFile(string_view path, const FileSystem &fs) {
    try {
        return fs.readFile(path);
    } catch (FileNotFoundException e) {
        // Act as if file is completely empty.
        // NOTE: It is not appropriate to throw an error here. Sorbet does not differentiate between Watchman
        // updates that specify if a file has changed or has been deleted, so this is the 'golden path' for deleted
        // files.
        // TODO(jvilk): Use Tombstone files instead.
        return "";
    }
}

class TerminateOnDestruction final {
    TaskQueue &queue;

public:
    TerminateOnDestruction(TaskQueue &queue) : queue{queue} {}
    ~TerminateOnDestruction() {
        absl::MutexLock lck(queue.getMutex());
        queue.terminate();
    }
};

CounterState mergeCounters(CounterState counters) {
    if (!counters.hasNullCounters()) {
        counterConsume(move(counters));
    }
    return getAndClearThreadCounters();
}

} // namespace

bool TaskQueue::isTerminated() const {
    return this->terminated;
}

void TaskQueue::terminate() {
    this->terminated = true;
}

bool TaskQueue::isPaused() const {
    return this->paused;
}

void TaskQueue::pause() {
    this->paused = true;
}

void TaskQueue::resume() {
    this->paused = false;
}

int TaskQueue::getErrorCode() const {
    return this->errorCode;
}

void TaskQueue::setErrorCode(int code) {
    this->errorCode = code;
}

CounterState &TaskQueue::getCounters() {
    return this->counters;
}

const std::deque<std::unique_ptr<LSPTask>> &TaskQueue::tasks() const {
    return this->pendingTasks;
}

std::deque<std::unique_ptr<LSPTask>> &TaskQueue::tasks() {
    return this->pendingTasks;
}

absl::Mutex *TaskQueue::getMutex() {
    return &this->stateMutex;
}

bool TaskQueue::ready() const {
    return this->terminated || (!this->paused && !this->pendingTasks.empty());
}

LSPPreprocessor::LSPPreprocessor(shared_ptr<LSPConfiguration> config, shared_ptr<TaskQueue> taskQueue,
                                 uint32_t initialVersion)
    : config(move(config)), taskQueue(std::move(taskQueue)), owner(this_thread::get_id()),
      nextVersion(initialVersion + 1) {}

string_view LSPPreprocessor::getFileContents(string_view path) const {
    auto maybeFileContents = maybeGetFileContents(path);
    if (!maybeFileContents.has_value()) {
        ENFORCE(false, "Editor sent a change request without a matching open request.");
        return string_view();
    }
    return maybeFileContents.value();
}

optional<string_view> LSPPreprocessor::maybeGetFileContents(string_view path) const {
    auto it = openFiles.find(path);
    optional<string_view> result;
    if (it == openFiles.end()) {
        return result;
    }
    result = it->second->source();
    return result;
}

void LSPPreprocessor::mergeFileChanges() {
    taskQueue->getMutex()->AssertHeld();
    auto &logger = config->logger;
    // mergeFileChanges is the most expensive operation this thread performs while holding the mutex lock.
    Timer timeit(logger, "lsp.mergeFileChanges");
    auto &pendingRequests = taskQueue->tasks();
    const int originalSize = pendingRequests.size();
    int requestsMergedCounter = 0;

    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto *olderEdit = dynamic_cast<SorbetWorkspaceEditTask *>(it->get());
        it++;
        if (olderEdit == nullptr) {
            // Is not an edit.
            continue;
        }
        // See which newer requests we can merge. We want to merge them *backwards* into olderEdit so we can complete
        // merging (and erase merged edits) in a single loop over the queue.
        while (it != pendingRequests.end()) {
            auto &task = *it;
            auto *newerEdit = dynamic_cast<SorbetWorkspaceEditTask *>(task.get());
            if (newerEdit == nullptr) {
                if (task->isDelayable()) {
                    ++it;
                    continue;
                } else {
                    // Stop merging edits into olderEdit if the pointed-to message is neither an edit nor delayable
                    break;
                }
            }
            olderEdit->mergeNewer(*newerEdit);
            // Delete the update we just merged and move on to next item.
            it = pendingRequests.erase(it);
            requestsMergedCounter++;
        }
    }
    ENFORCE(pendingRequests.size() + requestsMergedCounter == originalSize);
}

bool LSPPreprocessor::ensureInitialized(LSPMethod method, const LSPMessage &msg) const {
    // The following whitelisted methods are OK to run prior to LSP server initialization.
    // TODO(jvilk): Could encode this in each LSPTask.
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

unique_ptr<LSPTask> LSPPreprocessor::getTaskForMessage(LSPMessage &msg) {
    if (msg.isResponse()) {
        // We currently send no requests to the client, so we don't expect any response messages.
        return make_unique<SorbetErrorTask>(
            *config, make_unique<SorbetErrorParams>((int)LSPErrorCodes::MethodNotFound, "Unexpected response message"));
    }
    const LSPMethod method = msg.method();
    if (msg.isNotification()) {
        auto &rawParams = msg.asNotification().params;
        switch (method) {
            case LSPMethod::TextDocumentDidOpen: {
                auto &params = get<unique_ptr<DidOpenTextDocumentParams>>(rawParams);
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                return make_unique<SorbetWorkspaceEditTask>(*config, move(newParams));
            }
            case LSPMethod::TextDocumentDidClose: {
                auto &params = get<unique_ptr<DidCloseTextDocumentParams>>(rawParams);
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                return make_unique<SorbetWorkspaceEditTask>(*config, move(newParams));
            }
            case LSPMethod::TextDocumentDidChange: {
                auto &params = get<unique_ptr<DidChangeTextDocumentParams>>(rawParams);
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                return make_unique<SorbetWorkspaceEditTask>(*config, move(newParams));
            }
            case LSPMethod::SorbetWatchmanFileChange: {
                auto &params = get<unique_ptr<WatchmanQueryResponse>>(rawParams);
                auto newParams = canonicalizeEdits(nextVersion++, move(params));
                return make_unique<SorbetWorkspaceEditTask>(*config, move(newParams));
            }
            case LSPMethod::SorbetWatchmanStateEnter:
                return make_unique<WatchmanStateEnterTask>(*config,
                                                           move(get<unique_ptr<WatchmanStateEnter>>(rawParams)));
            case LSPMethod::SorbetWatchmanStateLeave:
                return make_unique<WatchmanStateLeaveTask>(*config,
                                                           move(get<unique_ptr<WatchmanStateLeave>>(rawParams)));
            case LSPMethod::SorbetWorkspaceEdit:
                return make_unique<SorbetWorkspaceEditTask>(
                    *config, move(get<unique_ptr<SorbetWorkspaceEditParams>>(rawParams)));
            case LSPMethod::$CancelRequest:
                return make_unique<CancelRequestTask>(*config, move(get<unique_ptr<CancelParams>>(rawParams)));
            case LSPMethod::Initialized:
                return make_unique<InitializedTask>(*config);
            case LSPMethod::Exit:
                return make_unique<ExitTask>(*config, 0);
            case LSPMethod::SorbetFence:
                return make_unique<SorbetFenceTask>(*config, move(get<unique_ptr<SorbetFenceParams>>(rawParams)));
            case LSPMethod::PAUSE:
                return make_unique<SorbetPauseTask>(*config);
            case LSPMethod::RESUME:
                return make_unique<SorbetResumeTask>(*config);
            case LSPMethod::SorbetError:
                return make_unique<SorbetErrorTask>(*config, move(get<unique_ptr<SorbetErrorParams>>(rawParams)));
            default:
                return make_unique<SorbetErrorTask>(
                    *config, make_unique<SorbetErrorParams>(
                                 (int)LSPErrorCodes::MethodNotFound,
                                 fmt::format("Unknown notification method: {}", convertLSPMethodToString(method))));
        }
    } else if (msg.isRequest()) {
        auto &requestMessage = msg.asRequest();
        // asRequest() should guarantee the presence of an ID.
        ENFORCE(msg.id());
        auto id = *msg.id();
        auto &rawParams = requestMessage.params;
        switch (method) {
            case LSPMethod::TextDocumentDefinition:
                return make_unique<DefinitionTask>(*config, id,
                                                   move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::Initialize:
                return make_unique<InitializeTask>(*config, id, move(get<unique_ptr<InitializeParams>>(rawParams)));
            case LSPMethod::TextDocumentDocumentHighlight:
                return make_unique<DocumentHighlightTask>(*config, id,
                                                          move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::TextDocumentDocumentSymbol:
                return make_unique<DocumentSymbolTask>(*config, id,
                                                       move(get<unique_ptr<DocumentSymbolParams>>(rawParams)));
            case LSPMethod::TextDocumentHover:
                return make_unique<HoverTask>(*config, id,
                                              move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::TextDocumentTypeDefinition:
                return make_unique<TypeDefinitionTask>(*config, id,
                                                       move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::WorkspaceSymbol:
                return make_unique<WorkspaceSymbolsTask>(*config, id,
                                                         move(get<unique_ptr<WorkspaceSymbolParams>>(rawParams)));
            case LSPMethod::TextDocumentCompletion:
                return make_unique<CompletionTask>(*config, id, move(get<unique_ptr<CompletionParams>>(rawParams)));
            case LSPMethod::TextDocumentCodeAction:
                return make_unique<CodeActionTask>(*config, id, move(get<unique_ptr<CodeActionParams>>(rawParams)));
            case LSPMethod::CodeActionResolve:
                return make_unique<CodeActionResolveTask>(*config, id, move(get<unique_ptr<CodeAction>>(rawParams)));
            case LSPMethod::TextDocumentFormatting:
                return make_unique<DocumentFormattingTask>(*config, id,
                                                           move(get<unique_ptr<DocumentFormattingParams>>(rawParams)));
            case LSPMethod::TextDocumentSignatureHelp:
                return make_unique<SignatureHelpTask>(*config, id,
                                                      move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::TextDocumentReferences:
                return make_unique<ReferencesTask>(*config, id, move(get<unique_ptr<ReferenceParams>>(rawParams)));
            case LSPMethod::TextDocumentImplementation:
                return make_unique<ImplementationTask>(*config, id,
                                                       move(get<unique_ptr<ImplementationParams>>(rawParams)));
            case LSPMethod::SorbetReadFile:
                return make_unique<SorbetReadFileTask>(*config, id,
                                                       move(get<unique_ptr<TextDocumentIdentifier>>(rawParams)));
            case LSPMethod::SorbetShowSymbol:
                return make_unique<SorbetShowSymbolTask>(*config, id,
                                                         move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::Shutdown:
                return make_unique<ShutdownTask>(*config, id);
            case LSPMethod::SorbetError:
                return make_unique<SorbetErrorTask>(*config, move(get<unique_ptr<SorbetErrorParams>>(rawParams)), id);
            case LSPMethod::GETCOUNTERS:
                return make_unique<GetCountersTask>(*config, id);
            case LSPMethod::TextDocumentPrepareRename:
                return make_unique<PrepareRenameTask>(*config, id,
                                                      move(get<unique_ptr<TextDocumentPositionParams>>(rawParams)));
            case LSPMethod::TextDocumentRename:
                return make_unique<RenameTask>(*config, id, move(get<unique_ptr<RenameParams>>(rawParams)));
            default:
                return make_unique<SorbetErrorTask>(
                    *config,
                    make_unique<SorbetErrorParams>(
                        (int)LSPErrorCodes::MethodNotFound,
                        fmt::format("Unknown request method: {}", convertLSPMethodToString(method))),
                    id);
        }
    } else {
        // This should be impossible.
        Exception::raise("Message isn't a request, notification, or response.");
    }
}

bool LSPPreprocessor::cancelRequest(const CancelParams &params) {
    absl::MutexLock lock(taskQueue->getMutex());
    auto &pendingTasks = taskQueue->tasks();
    for (auto it = pendingTasks.begin(); it != pendingTasks.end(); ++it) {
        auto &current = **it;
        if (current.cancel(params.id)) {
            pendingTasks.erase(it);
            // Now that we've removed this request, we may be able to merge more edits together.
            mergeFileChanges();
            return true;
        }
    }
    // It's too late; we have either already processed the request or are currently processing it. Swallow cancellation
    // and ignore.
    return false;
}

void LSPPreprocessor::pause() {
    absl::MutexLock lock(taskQueue->getMutex());
    ENFORCE(!taskQueue->isPaused());
    config->logger->error("Pausing");
    taskQueue->pause();
}

void LSPPreprocessor::resume() {
    absl::MutexLock lock(taskQueue->getMutex());
    ENFORCE(taskQueue->isPaused());
    config->logger->error("Resuming");
    taskQueue->resume();
}

void LSPPreprocessor::exit(int exitCode) {
    absl::MutexLock lock(taskQueue->getMutex());
    if (!taskQueue->isTerminated()) {
        taskQueue->terminate();
        taskQueue->setErrorCode(exitCode);
    }
}

void LSPPreprocessor::preprocessAndEnqueue(unique_ptr<LSPMessage> msg) {
    ENFORCE(owner == this_thread::get_id());
    if (msg->isResponse()) {
        // We ignore responses.
        return;
    }

    const LSPMethod method = msg->method();
    if (!ensureInitialized(method, *msg)) {
        // msg is invalid. Error sent to client.
        return;
    }

    auto task = getTaskForMessage(*msg);
    task->latencyTimer = move(msg->latencyTimer);

    {
        Timer timeit(config->logger, "LSPTask::preprocess");
        timeit.setTag("method", task->methodString());
        task->preprocess(*this);
    }
    if (task->finalPhase() != LSPTask::Phase::PREPROCESS) {
        // Enqueue task to be processed on processing thread.
        absl::MutexLock lock(taskQueue->getMutex());
        const bool isEdit = task->method == LSPMethod::SorbetWorkspaceEdit;
        taskQueue->tasks().push_back(move(task));
        if (isEdit) {
            // Only edits can be merged; avoid looping over the queue on every request.
            mergeFileChanges();
        }
    } else {
        prodCategoryCounterInc("lsp.messages.processed", task->methodString());
    }
}

unique_ptr<Joinable> LSPPreprocessor::runPreprocessor(MessageQueueState &messageQueue, absl::Mutex &messageQueueMutex) {
    return runInAThread("lspPreprocess", [this, &messageQueue, &messageQueueMutex] {
        // Propagate the termination flag across the two queues.
        MessageQueueState::NotifyOnDestruction notify(messageQueue, messageQueueMutex);
        TerminateOnDestruction notifyProcessing(*taskQueue);
        owner = this_thread::get_id();
        while (true) {
            unique_ptr<LSPMessage> msg;
            {
                absl::MutexLock lck(&messageQueueMutex);
                messageQueueMutex.Await(absl::Condition(
                    +[](MessageQueueState *messageQueue) -> bool {
                        return messageQueue->terminate || !messageQueue->pendingRequests.empty();
                    },
                    &messageQueue));
                // Only terminate once incoming queue is drained.
                if (messageQueue.terminate && messageQueue.pendingRequests.empty()) {
                    config->logger->debug("Preprocessor terminating");
                    return;
                }
                msg = move(messageQueue.pendingRequests.front());
                messageQueue.pendingRequests.pop_front();
                // Combine counters with this thread's counters.
                if (!messageQueue.counters.hasNullCounters()) {
                    counterConsume(move(messageQueue.counters));
                }
            }

            preprocessAndEnqueue(move(msg));

            {
                absl::MutexLock lck(taskQueue->getMutex());
                // Merge the counters from all of the worker threads with those stored in
                // taskQueue.
                taskQueue->getCounters() = mergeCounters(move(taskQueue->getCounters()));
                if (taskQueue->isTerminated()) {
                    // We must have processed an exit notification, or one of the downstream threads exited.
                    return;
                }
            }
        }
    });
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(uint32_t v, unique_ptr<DidChangeTextDocumentParams> changeParams) {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    edit->sorbetCancellationExpected = changeParams->sorbetCancellationExpected.value_or(false);
    edit->sorbetPreemptionsExpected = changeParams->sorbetPreemptionsExpected.value_or(0);
    string_view uri = changeParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            string fileContents = changeParams->getSource(getFileContents(localPath));
            auto fileType = core::File::Type::Normal;
            auto &slot = openFiles[localPath];
            auto file = make_shared<core::File>(move(localPath), move(fileContents), fileType, v);
            file->setIsOpenInClient(true);
            edit->updates.push_back(file);
            slot = move(file);
        }
    }
    return edit;
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(uint32_t v, unique_ptr<DidOpenTextDocumentParams> openParams) {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    string_view uri = openParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            auto fileType = core::File::Type::Normal;
            auto &slot = openFiles[localPath];
            auto file = make_shared<core::File>(move(localPath), move(openParams->textDocument->text), fileType, v);
            file->setIsOpenInClient(true);
            edit->updates.push_back(file);
            slot = move(file);
        }
    }
    return edit;
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(uint32_t v, unique_ptr<DidCloseTextDocumentParams> closeParams) {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    string_view uri = closeParams->textDocument->uri;
    if (config->isUriInWorkspace(uri)) {
        string localPath = config->remoteName2Local(uri);
        if (!config->isFileIgnored(localPath)) {
            openFiles.erase(localPath);
            // Use contents of file on disk.
            auto fileType = core::File::Type::Normal;
            auto fileContents = readFile(localPath, *config->opts.fs);
            edit->updates.push_back(make_shared<core::File>(move(localPath), move(fileContents), fileType, v));
        }
    }
    return edit;
}

unique_ptr<SorbetWorkspaceEditParams>
LSPPreprocessor::canonicalizeEdits(uint32_t v, unique_ptr<WatchmanQueryResponse> queryResponse) const {
    auto edit = make_unique<SorbetWorkspaceEditParams>();
    edit->epoch = v;
    for (auto &file : queryResponse->files) {
        // Don't append rootPath if it is empty.
        string localPath = !config->rootPath.empty() ? absl::StrCat(config->rootPath, "/", file) : file;
        // Editor contents supercede file system updates.
        if (!config->isFileIgnored(localPath) && !openFiles.contains(localPath)) {
            auto fileType = core::File::Type::Normal;
            auto fileContents = readFile(localPath, *config->opts.fs);
            edit->updates.push_back(make_shared<core::File>(move(localPath), move(fileContents), fileType, v));
        }
    }
    return edit;
}

} // namespace sorbet::realmain::lsp
