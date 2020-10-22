#include "main/lsp/LSPTask.h"
#include "absl/synchronization/notification.h"
#include "common/sort.h"
#include "core/NameHash.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTask::LSPTask(const LSPConfiguration &config, LSPMethod method) : config(config), method(method) {}

LSPTask::~LSPTask() {
    if (!latencyTimer) {
        return;
    }
    latencyTimer->setTag("method", methodString());
}

ConstExprStr LSPTask::methodString() const {
    switch (method) {
        case LSPMethod::$CancelRequest:
            return "cancelRequest";
        case LSPMethod::Exit:
            return "exit";
        case LSPMethod::PAUSE:
            return "PAUSE";
        case LSPMethod::RESUME:
            return "RESUME";
        case LSPMethod::GETCOUNTERS:
            return "GETCOUNTERS";
        case LSPMethod::Shutdown:
            return "shutdown";
        case LSPMethod::SorbetError:
            return "sorbet.error";
        case LSPMethod::SorbetFence:
            return "sorbet.fence";
        case LSPMethod::SorbetShowOperation:
        case LSPMethod::SorbetTypecheckRunInfo:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::WindowShowMessage:
            // Sorbet sends these to the client; the server shouldn't receive them.
            return "other";
        case LSPMethod::Initialize:
            return "initialize";
        case LSPMethod::Initialized:
            return "initialized";
        case LSPMethod::SorbetReadFile:
            return "sorbet.readFile";
        case LSPMethod::SorbetWatchmanFileChange:
            return "sorbet.watchmanFileChange";
        case LSPMethod::SorbetWorkspaceEdit:
        case LSPMethod::TextDocumentDidChange:
        case LSPMethod::TextDocumentDidClose:
        case LSPMethod::TextDocumentDidOpen:
            return "sorbet.workspaceEdit";
        case LSPMethod::TextDocumentCodeAction:
            return "textDocument.codeAction";
        case LSPMethod::TextDocumentCompletion:
            return "textDocument.completion";
        case LSPMethod::TextDocumentDefinition:
            return "textDocument.definition";
        case LSPMethod::TextDocumentDocumentHighlight:
            return "textDocument.documentHighlight";
        case LSPMethod::TextDocumentDocumentSymbol:
            return "textDocument.documentSymbol";
        case LSPMethod::TextDocumentFormatting:
            return "textDocument.formatting";
        case LSPMethod::TextDocumentHover:
            return "textDocument.hover";
        case LSPMethod::TextDocumentReferences:
            return "textDocument.references";
        case LSPMethod::TextDocumentSignatureHelp:
            return "textDocument.signatureHelp";
        case LSPMethod::TextDocumentTypeDefinition:
            return "textDocument.typeDefinition";
        case LSPMethod::WorkspaceSymbol:
            return "workspace.symbol";
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

LSPTask::Phase LSPRequestTask::finalPhase() const {
    return Phase::RUN;
}

bool LSPRequestTask::cancel(const MessageId &id) {
    if (this->id.equals(id)) {
        if (latencyTimer) {
            latencyTimer->cancel();
            latencyTimer = nullptr;
        }
        auto response = make_unique<ResponseMessage>("2.0", id, method);
        prodCategoryCounterInc("lsp.messages.canceled", methodString());
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
    for (auto &file : typechecker.state().getFiles()) {
        i++;
        if (file == nullptr) {
            continue;
        }

        ENFORCE(file->getFileHash() != nullptr);
        const auto &hash = *file->getFileHash();
        const auto &usedSends = hash.usages.sends;
        const auto &usedConstants = hash.usages.constants;
        auto ref = core::FileRef(i);

        const bool fileIsValid = ref.exists() && (ref.data(gs).sourceType == core::File::Type::Normal ||
                                                  ref.data(gs).sourceType == core::File::Type::Package);
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
                // 'queryBySymbolInFiles' may pick up secondary files required for accurate querying (e.g., package
                // files)
                if (location->uri == uri) {
                    auto highlight = make_unique<DocumentHighlight>(move(location->range));
                    highlights.push_back(move(highlight));
                }
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

            {
                Timer timeit(config.logger, "LSPTask::index");
                timeit.setTag("method", task->methodString());
                // Index while holding lock to prevent races with processing thread.
                task->index(indexer);
            }
        }
        prodCategoryCounterInc("lsp.messages.processed", task->methodString());

        if (task->finalPhase() == Phase::INDEX) {
            continue;
        }
        Timer timeit(config.logger, "LSPTask::run");
        timeit.setTag("method", task->methodString());
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
