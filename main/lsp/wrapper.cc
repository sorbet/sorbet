#include "main/lsp/wrapper.h"
#include "core/errors/namer.h"
#include "main/lsp/LSPInput.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include <memory>
#include <regex>

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
void fixGS(core::GlobalState &gs, options::Options &opts) {
    if (!opts.stripeMode) {
        // Definitions in multiple locations interact poorly with autoloader this error is enforced
        // in Stripe code.
        gs.suppressErrorClass(sorbet::core::errors::Namer::MultipleBehaviorDefs.code);
    }

    // If we don't tell the errorQueue to ignore flushes, then we won't get diagnostic messages.
    gs.errorQueue->ignoreFlushes = true;

    // Ensure LSP is enabled.
    opts.runLSP = true;
}

enum class FenceState {
    // A response to fenceStart has not yet been received.
    WAITING_FOR_START,
    // A response to fenceEnd has not yet been received.
    WAITING_FOR_END,
    DONE,
};

bool isFenceWithId(const LSPMessage &msg, int id) {
    return msg.isNotification() && msg.method() == LSPMethod::SorbetFence &&
           get<int>(msg.asNotification().params) == id;
}

unique_ptr<LSPMessage> makeFence(int id) {
    auto notif = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, id);
    return make_unique<LSPMessage>(move(notif));
}

/**
 * When the language server sends a message to the client, this output object passes that message to the provided lambda
 * function.
 *
 * Also supports collecting messages to the side that occur between two specific sorbet/fence requests. These will not
 * be sent to the lambda, but are instead returned via waitForCollectedMessages.
 */
class LSPLambdaOutput final : public LSPOutput {
    function<void(unique_ptr<LSPMessage>)> lambda GUARDED_BY(mtx);
    vector<unique_ptr<LSPMessage>> collectedMessages GUARDED_BY(mtx);
    // If this is not DONE, then we are in a special mode where we collect all messages between fenceStart and fenceEnd.
    FenceState collectedMessagesState GUARDED_BY(mtx) = FenceState::DONE;
    int fenceStart GUARDED_BY(mtx) = 0;
    int fenceEnd GUARDED_BY(mtx) = 0;

protected:
    void rawWrite(unique_ptr<LSPMessage> msg) override EXCLUSIVE_LOCKS_REQUIRED(mtx) {
        switch (collectedMessagesState) {
            case FenceState::WAITING_FOR_START:
                if (isFenceWithId(*msg, fenceStart)) {
                    collectedMessagesState = FenceState::WAITING_FOR_END;
                    // Don't send this fence (which is an internal implementation detail) back to the client.
                    return;
                }
                break;
            case FenceState::WAITING_FOR_END:
                if (isFenceWithId(*msg, fenceEnd)) {
                    collectedMessagesState = FenceState::DONE;
                    // Don't send this fence (which is an internal implementation detail) back to the client.
                    return;
                } else {
                    // Save message to the side; it's a message in-between the fences.
                    collectedMessages.push_back(move(msg));
                    // Don't send to lambda.
                    return;
                }
                break;
            default:
                break;
        }
        lambda(move(msg));
    }

public:
    LSPLambdaOutput(function<void(unique_ptr<LSPMessage>)> &&lambda) : lambda(move(lambda)) {}

    /**
     * Collect all messages received between fence with ID start and with ID end. Messages can later be retrieved with
     * `waitForCollectedMessages`.
     */
    void collectMessagesBetween(int fenceIdStart, int fenceIdEnd) {
        absl::MutexLock lock(&mtx);
        if (this->collectedMessagesState != FenceState::DONE || !this->collectedMessages.empty()) {
            Exception::raise(
                "Cannot call collectMessagesBetween a second time before calling waitForCollectedMessages.");
        }
        this->fenceStart = fenceIdStart;
        this->fenceEnd = fenceIdEnd;
        this->collectedMessagesState = FenceState::WAITING_FOR_START;
    }

    /**
     * Block until all collected messages are available.
     */
    vector<unique_ptr<LSPMessage>> waitForCollectedMessages() {
        absl::MutexLock lock(&mtx);
        mtx.Await(absl::Condition(
            +[](FenceState *collectedMessagesState) -> bool { return *collectedMessagesState == FenceState::DONE; },
            &collectedMessagesState));
        return move(collectedMessages);
    }
};

} // namespace

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(unique_ptr<LSPMessage> message) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(message));
    return getLSPResponsesFor(move(messages));
}

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(vector<unique_ptr<LSPMessage>> messages) {
    if (multithreadingEnabled) {
        auto config = dynamic_pointer_cast<LSPLambdaOutput>(output);
        ENFORCE(config != nullptr);
        const int fenceIdStart = 1;
        const int fenceIdEnd = 2;
        auto lambdaOutput = dynamic_pointer_cast<LSPLambdaOutput>(output);
        ENFORCE(lambdaOutput != nullptr);
        // Sandwich messages between two fences to isolate the responses to just these messages.
        lambdaOutput->collectMessagesBetween(fenceIdStart, fenceIdEnd);
        input->write(makeFence(fenceIdStart));
        input->write(move(messages));
        input->write(makeFence(fenceIdEnd));
        return lambdaOutput->waitForCollectedMessages();
    } else {
        lspLoop->processRequests(move(messages));
        return dynamic_pointer_cast<LSPOutputToVector>(output)->getOutput();
    }
}

vector<unique_ptr<LSPMessage>> LSPWrapper::getLSPResponsesFor(const string &json) {
    return getLSPResponsesFor(LSPMessage::fromClient(json));
}

void LSPWrapper::send(std::unique_ptr<LSPMessage> message) {
    if (!multithreadingEnabled) {
        Exception::raise("send is only possible in multithreaded mode. Use getLSPResponsesFor instead.");
    }
    input->write(move(message));
}

void LSPWrapper::send(std::vector<std::unique_ptr<LSPMessage>> &messages) {
    if (!multithreadingEnabled) {
        Exception::raise("send is only possible in multithreaded mode. Use getLSPResponsesFor instead.");
    }
    input->write(move(messages));
}

LSPWrapper::LSPWrapper(shared_ptr<options::Options> opts, shared_ptr<LSPLoop> lspLoop,
                       shared_ptr<LSPConfiguration> config, shared_ptr<LSPOutput> output, bool multithreadingEnabled,
                       unique_ptr<WorkerPool> workers, shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                       shared_ptr<spd::logger> typeErrorsConsole)
    : lspLoop(move(lspLoop)), config_(move(config)), output(move(output)), input(make_shared<LSPProgrammaticInput>()),
      workers(move(workers)), stderrColorSink(move(stderrColorSink)), typeErrorsConsole(move(typeErrorsConsole)),
      opts(move(opts)), multithreadingEnabled(multithreadingEnabled) {
    if (multithreadingEnabled) {
        if (emscripten_build) {
            Exception::raise("LSPWrapper cannot be used in multithreaded mode in Emscripten.");
        }
        // Start LSPLoop on a dedicated thread.
        lspThread = runInAThread("LSPLoop",
                                 [input = this->input, lspLoop = this->lspLoop]() -> void { lspLoop->runLSP(input); });
    }
}

// All factory constructors lead here. Separate from the actual constructor since const fields need to be initialized in
// initializer list.
unique_ptr<LSPWrapper> LSPWrapper::createInternal(unique_ptr<core::GlobalState> gs,
                                                  shared_ptr<options::Options> options,
                                                  const shared_ptr<spdlog::logger> &logger, bool disableFastPath,
                                                  optional<function<void(unique_ptr<LSPMessage>)>> &&processResponse,
                                                  shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                                                  shared_ptr<spd::logger> typeErrorsConsole) {
    fixGS(*gs, *options);
    auto workers = WorkerPool::create(options->threads, *logger);
    bool multithreadingEnabled = processResponse.has_value();
    shared_ptr<LSPOutput> output;
    if (multithreadingEnabled) {
        output = make_shared<LSPLambdaOutput>(move(processResponse.value()));
    } else {
        output = make_shared<LSPOutputToVector>();
    }
    // Configure LSPLoop to disable configatron.
    auto config = make_shared<LSPConfiguration>(*options, output, *workers, logger, true, disableFastPath);
    auto lspLoop = make_shared<LSPLoop>(std::move(gs), config);
    return make_unique<LSPWrapper>(move(options), move(lspLoop), move(config), move(output), multithreadingEnabled,
                                   move(workers), move(stderrColorSink), move(typeErrorsConsole));
}

unique_ptr<LSPWrapper> LSPWrapper::createInternal(string_view rootPath, shared_ptr<options::Options> options,
                                                  optional<function<void(unique_ptr<LSPMessage>)>> &&processResponse,
                                                  int numWorkerThreads, bool disableFastPath) {
    options->rawInputDirNames.emplace_back(rootPath);
    options->threads = numWorkerThreads;

    // All of this stuff is ignored by LSP, but we need it to construct ErrorQueue/GlobalState.
    auto stderrColorSink = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    auto logger = make_shared<spd::logger>("console", stderrColorSink);
    auto typeErrorsConsole = make_shared<spd::logger>("typeDiagnostics", stderrColorSink);
    typeErrorsConsole->set_pattern("%v");
    auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));
    unique_ptr<KeyValueStore> kvstore;
    payload::createInitialGlobalState(gs, *options, kvstore);

    return createInternal(move(gs), move(options), logger, disableFastPath, move(processResponse),
                          move(stderrColorSink), move(typeErrorsConsole));
}

unique_ptr<LSPWrapper> LSPWrapper::createSingleThreaded(unique_ptr<core::GlobalState> gs,
                                                        shared_ptr<options::Options> options,
                                                        const shared_ptr<spdlog::logger> &logger,
                                                        bool disableFastPath) {
    return createInternal(move(gs), move(options), logger, disableFastPath, nullopt);
}

unique_ptr<LSPWrapper> LSPWrapper::createSingleThreaded(string_view rootPath, shared_ptr<options::Options> options,
                                                        bool disableFastPath) {
    return createInternal(rootPath, move(options), nullopt, 0, disableFastPath);
}

unique_ptr<LSPWrapper> LSPWrapper::createMultiThreaded(function<void(unique_ptr<LSPMessage>)> &&processResponse,
                                                       string_view rootPath, shared_ptr<options::Options> options,
                                                       int numWorkerThreads, bool disableFastPath) {
    return createInternal(rootPath, move(options), move(processResponse), numWorkerThreads, disableFastPath);
}

LSPWrapper::~LSPWrapper() {
    if (multithreadingEnabled) {
        // End input stream so lsploop shuts down.
        input->close();
        // The destructor will wait until the lspThread Joinable destructs, which waits for thread death, which should
        // occur post-close.
    }
}

void LSPWrapper::enableAllExperimentalFeatures() {
    enableExperimentalFeature(LSPExperimentalFeature::Autocomplete);
    enableExperimentalFeature(LSPExperimentalFeature::DocumentHighlight);
    enableExperimentalFeature(LSPExperimentalFeature::DocumentSymbol);
    enableExperimentalFeature(LSPExperimentalFeature::SignatureHelp);
    enableExperimentalFeature(LSPExperimentalFeature::QuickFix);
}

void LSPWrapper::enableExperimentalFeature(LSPExperimentalFeature feature) {
    switch (feature) {
        case LSPExperimentalFeature::Autocomplete:
            opts->lspAutocompleteEnabled = true;
            break;
        case LSPExperimentalFeature::QuickFix:
            opts->lspQuickFixEnabled = true;
            break;
        case LSPExperimentalFeature::DocumentHighlight:
            opts->lspDocumentHighlightEnabled = true;
            break;
        case LSPExperimentalFeature::DocumentSymbol:
            opts->lspDocumentSymbolEnabled = true;
            break;
        case LSPExperimentalFeature::SignatureHelp:
            opts->lspSignatureHelpEnabled = true;
            break;
    }
}

int LSPWrapper::getTypecheckCount() {
    return lspLoop->getTypecheckCount();
}

const LSPConfiguration &LSPWrapper::config() const {
    return *config_;
}

} // namespace sorbet::realmain::lsp
