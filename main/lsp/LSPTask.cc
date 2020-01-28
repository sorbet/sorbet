#include "main/lsp/LSPTask.h"
#include "absl/synchronization/notification.h"
#include "common/sort.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/lsp.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTask::LSPTask(const LSPConfiguration &config, LSPMethod method) : config(config), method(method) {}

LSPTask::~LSPTask() {
    if (!latencyTimer) {
        return;
    }

    // The code below creates a new timer for this specific request that immediately gets reported.
    switch (method) {
        case LSPMethod::$CancelRequest:
        case LSPMethod::Exit:
        case LSPMethod::PAUSE:
        case LSPMethod::RESUME:
        case LSPMethod::Shutdown:
        case LSPMethod::SorbetError:
        case LSPMethod::SorbetFence:
        case LSPMethod::SorbetShowOperation:
        case LSPMethod::SorbetTypecheckRunInfo:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::WindowShowMessage:
            // Not a request we care about. Bucket it in case it gets large.
            latencyTimer->fork("latency.other");
            break;
        case LSPMethod::Initialize:
            latencyTimer->fork("latency.initialize");
            break;
        case LSPMethod::Initialized:
            latencyTimer->fork("latency.initialized");
            break;
        case LSPMethod::SorbetReadFile:
            latencyTimer->fork("latency.sorbetreadfile");
            break;
        case LSPMethod::SorbetWatchmanFileChange:
        case LSPMethod::SorbetWorkspaceEdit:
        case LSPMethod::TextDocumentDidChange:
        case LSPMethod::TextDocumentDidClose:
        case LSPMethod::TextDocumentDidOpen:
            latencyTimer->fork("latency.fileedit");
            break;
        case LSPMethod::TextDocumentCodeAction:
            latencyTimer->fork("latency.codeaction");
            break;
        case LSPMethod::TextDocumentCompletion:
            latencyTimer->fork("latency.completion");
            break;
        case LSPMethod::TextDocumentDefinition:
            latencyTimer->fork("latency.definition");
            break;
        case LSPMethod::TextDocumentDocumentHighlight:
            latencyTimer->fork("latency.documenthighlight");
            break;
        case LSPMethod::TextDocumentDocumentSymbol:
            latencyTimer->fork("latency.documentsymbol");
            break;
        case LSPMethod::TextDocumentHover:
            latencyTimer->fork("latency.hover");
            break;
        case LSPMethod::TextDocumentReferences:
            latencyTimer->fork("latency.references");
            break;
        case LSPMethod::TextDocumentSignatureHelp:
            latencyTimer->fork("latency.signaturehelp");
            break;
        case LSPMethod::TextDocumentTypeDefinition:
            latencyTimer->fork("latency.typedefinition");
            break;
        case LSPMethod::WorkspaceSymbol:
            latencyTimer->fork("latency.workspacesymbol");
            break;
    }
}

LSPTask::Phase LSPTask::finalPhase() const {
    return Phase::RUN;
}

void LSPTask::preprocess(LSPPreprocessor &preprocessor) {}

void LSPTask::index(LSPIndexer &indexer) {}

LSPRequestTask::LSPRequestTask(const LSPConfiguration &config, MessageId id, LSPMethod method)
    : LSPTask(config, method), id(move(id)) {}

void LSPRequestTask::run(LSPTypecheckerDelegate &typechecker) {
    auto response = runRequest(typechecker);
    ENFORCE(response != nullptr);
    config.output->write(move(response));
}

bool LSPRequestTask::cancel(const MessageId &id) {
    if (this->id.equals(id)) {
        // Don't report a latency metric for canceled requests.
        if (latencyTimer) {
            latencyTimer->cancel();
        }
        auto response = make_unique<ResponseMessage>("2.0", id, method);
        prodCounterInc("lsp.messages.canceled");
        response->error = make_unique<ResponseError>((int)LSPErrorCodes::RequestCancelled, "Request was canceled");
        config.output->write(move(response));
        return true;
    }
    return false;
}

LSPQueryResult LSPTask::queryByLoc(LSPTypecheckerDelegate &typechecker, string_view uri, const Position &pos,
                                   const LSPMethod forMethod, bool errorIfFileIsUntyped) const {
    Timer timeit(config.logger, "setupLSPQueryByLoc");
    const core::GlobalState &gs = typechecker.state();
    auto fref = config.uri2FileRef(gs, uri);
    if (!fref.exists()) {
        auto error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidParams,
            fmt::format("Did not find file at uri {} in {}", uri, convertLSPMethodToString(forMethod)));
        return LSPQueryResult{{}, move(error)};
    }

    if (errorIfFileIsUntyped && fref.data(gs).strictLevel < core::StrictLevel::True) {
        config.logger->info("Ignoring request on untyped file `{}`", uri);
        // Act as if the query returned no results.
        return LSPQueryResult{{}};
    }

    auto loc = config.lspPos2Loc(fref, pos, gs);
    return typechecker.query(core::lsp::Query::createLocQuery(loc), {fref});
}

LSPQueryResult LSPTask::queryBySymbolInFiles(LSPTypecheckerDelegate &typechecker, core::SymbolRef sym,
                                             vector<core::FileRef> frefs) const {
    Timer timeit(config.logger, "setupLSPQueryBySymbolInFiles");
    ENFORCE(sym.exists());
    return typechecker.query(core::lsp::Query::createSymbolQuery(sym), frefs);
}

LSPQueryResult LSPTask::queryBySymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef sym) const {
    Timer timeit(config.logger, "setupLSPQueryBySymbol");
    ENFORCE(sym.exists());
    vector<core::FileRef> frefs;
    const core::GlobalState &gs = typechecker.state();
    const core::NameHash symNameHash(gs, sym.data(gs)->name.data(gs));
    // Locate files that contain the same Name as the symbol. Is an overapproximation, but a good first filter.
    int i = -1;
    for (auto &hash : typechecker.getFileHashes()) {
        i++;
        const auto &usedSends = hash.usages.sends;
        const auto &usedConstants = hash.usages.constants;
        auto ref = core::FileRef(i);

        const bool fileIsValid = ref.exists() && ref.data(gs).sourceType == core::File::Type::Normal;
        if (fileIsValid &&
            (std::find(usedSends.begin(), usedSends.end(), symNameHash) != usedSends.end() ||
             std::find(usedConstants.begin(), usedConstants.end(), symNameHash) != usedConstants.end())) {
            frefs.emplace_back(ref);
        }
    }

    return typechecker.query(core::lsp::Query::createSymbolQuery(sym), frefs);
}

bool LSPTask::needsMultithreading(const LSPIndexer &indexer) const {
    return false;
}

bool LSPTask::isDelayable() const {
    return false;
}

bool LSPTask::cancel(const MessageId &id) {
    return false;
}

bool LSPTask::canPreempt(const LSPIndexer &indexer) const {
    // A task that can preempt cannot be multithreaded.
    return !needsMultithreading(indexer);
}

vector<unique_ptr<Location>>
LSPTask::extractLocations(const core::GlobalState &gs,
                          const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                          vector<unique_ptr<Location>> locations) const {
    for (auto &q : queryResponses) {
        core::Loc loc = q->getLoc();
        if (loc.exists() && loc.file().exists()) {
            auto fileIsTyped = loc.file().data(gs).strictLevel >= core::StrictLevel::True;
            // If file is untyped, only support responses involving constants and definitions.
            if (fileIsTyped || q->isConstant() || q->isField() || q->isDefinition()) {
                addLocIfExists(gs, locations, loc);
            }
        }
    }
    // Dedupe locations
    fast_sort(locations,
              [](const unique_ptr<Location> &a, const unique_ptr<Location> &b) -> bool { return a->cmp(*b) < 0; });
    locations.resize(std::distance(locations.begin(),
                                   std::unique(locations.begin(), locations.end(),
                                               [](const unique_ptr<Location> &a,
                                                  const unique_ptr<Location> &b) -> bool { return a->cmp(*b) == 0; })));
    return locations;
}

vector<unique_ptr<Location>> LSPTask::getReferencesToSymbol(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol,
                                                            vector<unique_ptr<Location>> locations) const {
    if (symbol.exists()) {
        auto run2 = queryBySymbol(typechecker, symbol);
        locations = extractLocations(typechecker.state(), run2.responses, move(locations));
    }
    return locations;
}

vector<unique_ptr<DocumentHighlight>>
LSPTask::getHighlightsToSymbolInFile(LSPTypecheckerDelegate &typechecker, string_view const uri, core::SymbolRef symbol,
                                     vector<unique_ptr<DocumentHighlight>> highlights) const {
    if (symbol.exists()) {
        auto fref = config.uri2FileRef(typechecker.state(), uri);
        if (fref.exists()) {
            auto run2 = queryBySymbolInFiles(typechecker, symbol, {fref});
            auto locations = extractLocations(typechecker.state(), run2.responses);
            for (auto const &location : locations) {
                auto highlight = make_unique<DocumentHighlight>(move(location->range));
                highlights.push_back(move(highlight));
            }
        }
    }
    return highlights;
}

void LSPTask::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<Location>> &locs, core::Loc loc) const {
    auto location = config.loc2Location(gs, loc);
    if (location != nullptr) {
        locs.push_back(std::move(location));
    }
}

LSPQueuePreemptionTask::LSPQueuePreemptionTask(const LSPConfiguration &config, absl::Notification &finished,
                                               absl::Mutex &taskQueueMutex, TaskQueueState &taskQueue,
                                               LSPIndexer &indexer)
    : LSPTask(config, LSPMethod::SorbetError), finished(finished), taskQueueMutex(taskQueueMutex), taskQueue(taskQueue),
      indexer(indexer) {}

void LSPQueuePreemptionTask::run(LSPTypecheckerDelegate &tc) {
    for (;;) {
        unique_ptr<LSPTask> task;
        {
            absl::MutexLock lck(&taskQueueMutex);
            if (taskQueue.pendingTasks.empty() || !taskQueue.pendingTasks.front()->canPreempt(indexer)) {
                break;
            }
            task = move(taskQueue.pendingTasks.front());
            taskQueue.pendingTasks.pop_front();

            // Index while holding lock to prevent races with processing thread.
            task->index(indexer);
        }
        prodCounterInc("lsp.messages.received");

        if (task->finalPhase() == Phase::INDEX) {
            continue;
        }
        task->run(tc);
    }
    finished.Notify();
}

LSPDangerousTypecheckerTask::LSPDangerousTypecheckerTask(const LSPConfiguration &config, LSPMethod method)
    : LSPTask(config, method) {}

void LSPDangerousTypecheckerTask::run(LSPTypecheckerDelegate &tc) {
    Exception::raise("Bug: Dangerous typechecker tasks are expected to run specially");
}

} // namespace sorbet::realmain::lsp