#include "main/lsp/lsp.h"
#include "common/Timer.h"
#include "common/statsd/statsd.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "main/options/options.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(unique_ptr<core::GlobalState> gs, const options::Options &opts,
                 const shared_ptr<spdlog::logger> &logger, WorkerPool &workers, int inputFd, std::ostream &outputStream,
                 bool skipConfigatron, bool disableFastPath)
    : opts(opts), logger(logger), inputFd(inputFd), outputStream(outputStream),
      lastMetricUpdateTime(chrono::steady_clock::now()),
      state(move(gs), logger, opts, workers, skipConfigatron, disableFastPath) {
    if (opts.rawInputDirNames.size() != 1) {
        logger->error("Sorbet's language server requires a single input directory.");
        throw options::EarlyReturnWithCode(1);
    }
    rootPath = opts.rawInputDirNames.at(0);
}

variant<TypecheckRun, pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>
LSPLoop::setupLSPQueryByLoc(unique_ptr<core::GlobalState> gs, string_view uri, const Position &pos,
                            const LSPMethod forMethod, bool errorIfFileIsUntyped) {
    Timer timeit(logger, "setupLSPQueryByLoc");
    absl::MutexLock lock(&state.mtx);
    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        return make_pair(make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams,
                                                    fmt::format("Did not find file at uri {} in {}", uri,
                                                                convertLSPMethodToString(forMethod))),
                         move(gs));
    }

    if (errorIfFileIsUntyped && fref.data(*gs).strictLevel < core::StrictLevel::Typed) {
        logger->info("Ignoring request on untyped file `{}`", uri);
        // Act as if the query returned no results.
        return TypecheckRun{{}, {}, {}, move(gs)};
    }
    auto loc = lspPos2Loc(fref, pos, *gs);
    if (!loc) {
        return make_pair(make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams,
                                                    fmt::format("Did not find location at uri {} in {}", uri,
                                                                convertLSPMethodToString(forMethod))),
                         move(gs));
    }

    vector<shared_ptr<core::File>> files;
    files.emplace_back(fref.data(*gs).deepCopy(*gs));
    return state.runLSPQuery(move(gs), core::lsp::Query::createLocQuery(*loc.get()), files);
}

TypecheckRun LSPLoop::setupLSPQueryBySymbol(unique_ptr<core::GlobalState> gs, core::SymbolRef sym) {
    Timer timeit(logger, "setupLSPQueryBySymbol");
    absl::MutexLock lock(&state.mtx);
    ENFORCE(sym.exists());
    vector<shared_ptr<core::File>> files;
    return state.runLSPQuery(move(gs), core::lsp::Query::createSymbolQuery(sym), files, true);
}

bool LSPLoop::ensureInitialized(LSPMethod forMethod, const LSPMessage &msg,
                                const unique_ptr<core::GlobalState> &currentGs) {
    if (initialized || forMethod == LSPMethod::Initialize || forMethod == LSPMethod::Initialized ||
        forMethod == LSPMethod::Exit || forMethod == LSPMethod::Shutdown || forMethod == LSPMethod::SorbetError) {
        return true;
    }
    return false;
}

LSPResult LSPLoop::pushDiagnostics(TypecheckRun run) {
    const core::GlobalState &gs = *run.gs;
    const auto &filesTypechecked = run.filesTypechecked;
    vector<core::FileRef> errorFilesInNewRun;
    UnorderedMap<core::FileRef, vector<std::unique_ptr<core::Error>>> errorsAccumulated;
    vector<unique_ptr<LSPMessage>> responses;

    for (auto &e : run.errors) {
        if (e->isSilenced) {
            continue;
        }
        auto file = e->loc.file();
        errorsAccumulated[file].emplace_back(std::move(e));
    }

    for (auto &accumulated : errorsAccumulated) {
        errorFilesInNewRun.push_back(accumulated.first);
    }

    vector<core::FileRef> filesToUpdateErrorListFor = errorFilesInNewRun;

    UnorderedSet<core::FileRef> filesTypecheckedAsSet;
    filesTypecheckedAsSet.insert(filesTypechecked.begin(), filesTypechecked.end());

    for (auto f : this->filesThatHaveErrors) {
        if (filesTypecheckedAsSet.find(f) != filesTypecheckedAsSet.end()) {
            // we've retypechecked this file. We can override the fact it has an error
            // thus, we will update the error list for this file on client
            filesToUpdateErrorListFor.push_back(f);
        } else {
            // we're not typecking this file, we need to remember that it had error
            errorFilesInNewRun.push_back(f);
        }
    }

    fast_sort(filesToUpdateErrorListFor);
    filesToUpdateErrorListFor.erase(unique(filesToUpdateErrorListFor.begin(), filesToUpdateErrorListFor.end()),
                                    filesToUpdateErrorListFor.end());

    fast_sort(errorFilesInNewRun);
    errorFilesInNewRun.erase(unique(errorFilesInNewRun.begin(), errorFilesInNewRun.end()), errorFilesInNewRun.end());

    this->filesThatHaveErrors = errorFilesInNewRun;

    for (auto file : filesToUpdateErrorListFor) {
        if (file.exists()) {
            string uri;
            { // uri
                if (file.data(gs).sourceType == core::File::Type::Payload) {
                    uri = string(file.data(gs).path());
                } else {
                    uri = localName2Remote(file.data(gs).path());
                }
            }

            vector<unique_ptr<Diagnostic>> diagnostics;
            {
                // diagnostics
                if (errorsAccumulated.find(file) != errorsAccumulated.end()) {
                    for (auto &e : errorsAccumulated[file]) {
                        auto diagnostic = make_unique<Diagnostic>(loc2Range(gs, e->loc), e->header);
                        diagnostic->code = e->what.code;
                        diagnostic->severity = DiagnosticSeverity::Error;

                        typecase(e.get(), [&](core::Error *ce) {
                            vector<unique_ptr<DiagnosticRelatedInformation>> relatedInformation;
                            for (auto &section : ce->sections) {
                                string sectionHeader = section.header;

                                for (auto &errorLine : section.messages) {
                                    string message;
                                    if (errorLine.formattedMessage.length() > 0) {
                                        message = errorLine.formattedMessage;
                                    } else {
                                        message = sectionHeader;
                                    }
                                    relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(
                                        loc2Location(gs, errorLine.loc), message));
                                }
                            }
                            diagnostic->relatedInformation = move(relatedInformation);
                        });
                        diagnostics.push_back(move(diagnostic));
                    }
                }
            }

            responses.push_back(make_unique<LSPMessage>(
                make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentPublishDiagnostics,
                                                 make_unique<PublishDiagnosticsParams>(uri, move(diagnostics)))));
        }
    }
    return LSPResult{move(run.gs), move(responses)};
}

constexpr chrono::minutes STATSD_INTERVAL = chrono::minutes(5);

bool LSPLoop::shouldSendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) {
    return !opts.statsdHost.empty() && (currentTime - lastMetricUpdateTime) > STATSD_INTERVAL;
}

void LSPLoop::sendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) {
    ENFORCE(this_thread::get_id() == mainThreadId, "sendCounterToStatsd can only be called from the main LSP thread.");
    // Record rusage-related stats.
    StatsD::addRusageStats();
    auto counters = getAndClearThreadCounters();
    if (!opts.statsdHost.empty()) {
        lastMetricUpdateTime = currentTime;

        auto prefix = fmt::format("{}.lsp.counters", opts.statsdPrefix);
        StatsD::submitCounters(counters, opts.statsdHost, opts.statsdPort, prefix);
    }
    if (!opts.webTraceFile.empty()) {
        web_tracer_framework::Tracing::storeTraces(counters, opts.webTraceFile);
    }
}

LSPResult LSPResult::make(unique_ptr<core::GlobalState> gs, unique_ptr<ResponseMessage> response) {
    LSPResult rv{move(gs), {}};
    rv.responses.push_back(make_unique<LSPMessage>(move(response)));
    return rv;
}

} // namespace sorbet::realmain::lsp
