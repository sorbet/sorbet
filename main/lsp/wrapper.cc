#include "main/lsp/wrapper.h"
#include "core/ErrorQueue.h"
#include "core/NullFlusher.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "main/cache/cache.h"
#include "main/lsp/LSPInput.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPOutput.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "sorbet_version/sorbet_version.h"
#include <memory>
#include <regex>

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
void setRequiredLSPOptions(core::GlobalState &gs, options::Options &options) {
    if (!options.stripeMode) {
        // Definitions in multiple locations interact poorly with autoloader this error is enforced
        // in Stripe code.
        gs.suppressErrorClass(sorbet::core::errors::Namer::MultipleBehaviorDefs.code);
    }

    gs.requiresAncestorEnabled = options.requiresAncestorEnabled;
    gs.ruby3KeywordArgs = options.ruby3KeywordArgs;
    gs.lspExperimentalFastPathEnabled = options.lspExperimentalFastPathEnabled;

    // Ensure LSP is enabled.
    options.runLSP = true;
}

pair<unique_ptr<core::GlobalState>, unique_ptr<KeyValueStore>>
createGlobalStateAndOtherObjects(string_view rootPath, options::Options &options, int numWorkerThreads,
                                 shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> &stderrColorSinkOut,
                                 shared_ptr<spd::logger> &loggerOut, shared_ptr<spd::logger> &typeErrorsConsoleOut) {
    options.rawInputDirNames.emplace_back(rootPath);
    options.threads = numWorkerThreads;

    // All of this stuff is ignored by LSP, but we need it to construct ErrorQueue/GlobalState.
    stderrColorSinkOut = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    loggerOut = make_shared<spd::logger>("console", stderrColorSinkOut);
    typeErrorsConsoleOut = make_shared<spd::logger>("typeDiagnostics", stderrColorSinkOut);
    typeErrorsConsoleOut->set_pattern("%v");
    auto gs = make_unique<core::GlobalState>(make_shared<core::ErrorQueue>(*typeErrorsConsoleOut, *loggerOut));

    unique_ptr<const OwnedKeyValueStore> kvstore = cache::maybeCreateKeyValueStore(loggerOut, options);
    payload::createInitialGlobalState(gs, options, kvstore);
    setRequiredLSPOptions(*gs, options);
    return make_pair(move(gs), OwnedKeyValueStore::abort(move(kvstore)));
}

} // namespace

vector<unique_ptr<LSPMessage>> SingleThreadedLSPWrapper::getLSPResponsesFor(unique_ptr<LSPMessage> message) {
    lspLoop->processRequest(move(message));
    return output->getOutput();
}

vector<unique_ptr<LSPMessage>> SingleThreadedLSPWrapper::getLSPResponsesFor(vector<unique_ptr<LSPMessage>> messages) {
    lspLoop->processRequests(move(messages));
    return output->getOutput();
}

vector<unique_ptr<LSPMessage>> SingleThreadedLSPWrapper::getLSPResponsesFor(const string &json) {
    return getLSPResponsesFor(LSPMessage::fromClient(json));
}

void MultiThreadedLSPWrapper::send(std::unique_ptr<LSPMessage> message) {
    input->write(move(message));
}

void MultiThreadedLSPWrapper::send(std::vector<std::unique_ptr<LSPMessage>> &messages) {
    input->write(move(messages));
}

void MultiThreadedLSPWrapper::send(const std::string &json) {
    send(LSPMessage::fromClient(json));
}

LSPWrapper::LSPWrapper(unique_ptr<core::GlobalState> gs, shared_ptr<options::Options> opts,
                       std::shared_ptr<spd::logger> logger,
                       shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                       shared_ptr<spd::logger> typeErrorsConsole, unique_ptr<KeyValueStore> kvstore,
                       bool disableFastPath)
    : logger(logger), workers(WorkerPool::create(opts->threads, *logger)), stderrColorSink(move(stderrColorSink)),
      typeErrorsConsole(move(typeErrorsConsole)), output(make_shared<LSPOutputToVector>()),
      config_(make_shared<LSPConfiguration>(*opts, output, move(logger), disableFastPath)),
      lspLoop(make_shared<LSPLoop>(std::move(gs), *workers, config_, move(kvstore))), opts(move(opts)) {}

LSPWrapper::~LSPWrapper() = default;

SingleThreadedLSPWrapper::SingleThreadedLSPWrapper(unique_ptr<core::GlobalState> gs, shared_ptr<options::Options> opts,
                                                   shared_ptr<spd::logger> logger,
                                                   shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                                                   shared_ptr<spd::logger> typeErrorsConsole,
                                                   unique_ptr<KeyValueStore> kvstore, bool disableFastPath)
    : LSPWrapper(move(gs), move(opts), move(logger), move(stderrColorSink), move(typeErrorsConsole), move(kvstore),
                 disableFastPath) {}

MultiThreadedLSPWrapper::MultiThreadedLSPWrapper(unique_ptr<core::GlobalState> gs, shared_ptr<options::Options> opts,
                                                 shared_ptr<spd::logger> logger,
                                                 shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink,
                                                 shared_ptr<spd::logger> typeErrorsConsole,
                                                 unique_ptr<KeyValueStore> kvstore, bool disableFastPath)
    : LSPWrapper(move(gs), move(opts), move(logger), move(stderrColorSink), move(typeErrorsConsole), move(kvstore),
                 disableFastPath),
      input(make_shared<LSPProgrammaticInput>()),
      lspThread(runInAThread("LSPWrapper", [&]() -> void { lspLoop->runLSP(input); })) {}

MultiThreadedLSPWrapper::~MultiThreadedLSPWrapper() {
    // Closing the input causes the lspLoop thread to terminate.
    input->close();
    // The default destructor behavior will cause this method to pause until the thread terminates, as the destructor of
    // lspThread implements that behavior.
}

unique_ptr<SingleThreadedLSPWrapper>
SingleThreadedLSPWrapper::createWithGlobalState(unique_ptr<core::GlobalState> gs, shared_ptr<options::Options> options,
                                                shared_ptr<spdlog::logger> logger,
                                                std::unique_ptr<KeyValueStore> kvstore, bool disableFastPath) {
    setRequiredLSPOptions(*gs, *options);
    // Note: To keep the constructor private, we need to construct with `new` and put it into a `unique_ptr` privately.
    // `make_unique` doesn't work because that method doesn't have access to the constructor.
    auto wrapper = new SingleThreadedLSPWrapper(move(gs), move(options), move(logger), nullptr, nullptr, move(kvstore),
                                                disableFastPath);
    return unique_ptr<SingleThreadedLSPWrapper>(wrapper);
}

unique_ptr<SingleThreadedLSPWrapper>
SingleThreadedLSPWrapper::create(string_view rootPath, shared_ptr<options::Options> options, bool disableFastPath) {
    shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
    shared_ptr<spd::logger> logger;
    shared_ptr<spd::logger> typeErrorsConsole;
    auto pair = createGlobalStateAndOtherObjects(rootPath, *options, 0, stderrColorSink, logger, typeErrorsConsole);
    // See comment in `SingleThreadedLSPWrapper::createWithGlobalState`
    auto wrapper = new SingleThreadedLSPWrapper(move(pair.first), move(options), move(logger), move(stderrColorSink),
                                                move(typeErrorsConsole), move(pair.second), disableFastPath);
    return unique_ptr<SingleThreadedLSPWrapper>(wrapper);
}

unique_ptr<MultiThreadedLSPWrapper> MultiThreadedLSPWrapper::create(string_view rootPath,
                                                                    shared_ptr<options::Options> options,
                                                                    int numWorkerThreads, bool disableFastPath) {
    shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
    shared_ptr<spd::logger> logger;
    shared_ptr<spd::logger> typeErrorsConsole;
    auto pair = createGlobalStateAndOtherObjects(rootPath, *options, numWorkerThreads, stderrColorSink, logger,
                                                 typeErrorsConsole);
    // See comment in `SingleThreadedLSPWrapper::createWithGlobalState`
    auto wrapper = new MultiThreadedLSPWrapper(move(pair.first), move(options), move(logger), move(stderrColorSink),
                                               move(typeErrorsConsole), move(pair.second), disableFastPath);
    return unique_ptr<MultiThreadedLSPWrapper>(wrapper);
}

unique_ptr<LSPMessage> MultiThreadedLSPWrapper::read(int timeoutMs) {
    return output->read(timeoutMs);
}

void LSPWrapper::enableAllExperimentalFeatures() {
    enableExperimentalFeature(LSPExperimentalFeature::DocumentHighlight);
    enableExperimentalFeature(LSPExperimentalFeature::DocumentSymbol);
    enableExperimentalFeature(LSPExperimentalFeature::SignatureHelp);
    enableExperimentalFeature(LSPExperimentalFeature::DocumentFormat);
    enableExperimentalFeature(LSPExperimentalFeature::ExperimentalFastPath);
}

void LSPWrapper::enableExperimentalFeature(LSPExperimentalFeature feature) {
    switch (feature) {
        case LSPExperimentalFeature::DocumentHighlight:
            opts->lspDocumentHighlightEnabled = true;
            break;
        case LSPExperimentalFeature::DocumentSymbol:
            opts->lspDocumentSymbolEnabled = true;
            break;
        case LSPExperimentalFeature::SignatureHelp:
            opts->lspSignatureHelpEnabled = true;
            break;
        case LSPExperimentalFeature::DocumentFormat:
            opts->lspDocumentFormatRubyfmtEnabled = true;
            break;
        case LSPExperimentalFeature::ExperimentalFastPath:
            opts->lspExperimentalFastPathEnabled = true;
            break;
    }
}

int LSPWrapper::getTypecheckCount() {
    return lspLoop->getTypecheckCount();
}

void LSPWrapper::setSlowPathBlocked(bool blocked) {
    lspLoop->setSlowPathBlocked(blocked);
}

bool LSPWrapper::isSlowPathBlocked() const {
    return lspLoop->isSlowPathBlocked();
}

const LSPConfiguration &LSPWrapper::config() const {
    return *config_;
}

} // namespace sorbet::realmain::lsp
