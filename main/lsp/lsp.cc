#include "main/lsp/lsp.h"
#include "common/Timer.h"
#include "common/statsd/statsd.h"
#include "common/typecase.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(unique_ptr<core::GlobalState> gs, const options::Options &opts, const shared_ptr<spd::logger> &logger,
                 WorkerPool &workers, int inputFd, std::ostream &outputStream, bool skipConfigatron,
                 bool disableFastPath)
    : initialGS(std::move(gs)), opts(opts), logger(logger), workers(workers), inputFd(inputFd),
      outputStream(outputStream), skipConfigatron(skipConfigatron), disableFastPath(disableFastPath),
      lastMetricUpdateTime(chrono::steady_clock::now()) {
    errorQueue = dynamic_pointer_cast<core::ErrorQueue>(initialGS->errorQueue);
    ENFORCE(errorQueue, "LSPLoop got an unexpected error queue");
    ENFORCE(errorQueue->ignoreFlushes,
            "LSPLoop's error queue is not ignoring flushes, which will prevent LSP from sending diagnostics");

    if (opts.rawInputDirNames.size() != 1) {
        logger->error("Sorbet's language server requires a single input directory.");
        throw options::EarlyReturnWithCode(1);
    }
    rootPath = opts.rawInputDirNames.at(0);
}

variant<LSPLoop::TypecheckRun, pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>
LSPLoop::setupLSPQueryByLoc(unique_ptr<core::GlobalState> gs, string_view uri, const Position &pos,
                            const LSPMethod forMethod, bool errorIfFileIsUntyped) const {
    Timer timeit(logger, "setupLSPQueryByLoc");
    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        return make_pair(make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams,
                                                    fmt::format("Did not find file at uri {} in {}", uri,
                                                                convertLSPMethodToString(forMethod))),
                         move(gs));
    }

    if (errorIfFileIsUntyped && fref.data(*gs).strictLevel < core::StrictLevel::True) {
        logger->info("Ignoring request on untyped file `{}`", uri);
        // Act as if the query returned no results.
        return TypecheckRun{{}, {}, {}, move(gs), {}, true};
    }

    auto loc = lspPos2Loc(fref, pos, *gs);
    if (!loc) {
        return make_pair(make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams,
                                                    fmt::format("Did not find location at uri {} in {}", uri,
                                                                convertLSPMethodToString(forMethod))),
                         move(gs));
    }

    return tryFastPath(move(gs), {}, {fref}, core::lsp::Query::createLocQuery(*loc.get()));
}

LSPLoop::TypecheckRun LSPLoop::setupLSPQueryBySymbol(unique_ptr<core::GlobalState> gs, core::SymbolRef sym) const {
    Timer timeit(logger, "setupLSPQueryBySymbol");
    ENFORCE(sym.exists());
    vector<core::FileRef> frefs;
    const core::NameHash symNameHash(*gs, sym.data(*gs)->name.data(*gs));
    // Locate files that contain the same Name as the symbol. Is an overapproximation, but a good first filter.
    int i = -1;
    for (auto &hash : globalStateHashes) {
        i++;
        const auto &usedSends = hash.usages.sends;
        const auto &usedConstants = hash.usages.constants;
        auto ref = core::FileRef(i);

        const bool fileIsValid = ref.exists() && ref.data(*gs).sourceType == core::File::Type::Normal;
        if (fileIsValid &&
            (std::find(usedSends.begin(), usedSends.end(), symNameHash) != usedSends.end() ||
             std::find(usedConstants.begin(), usedConstants.end(), symNameHash) != usedConstants.end())) {
            frefs.emplace_back(ref);
        }
    }

    return tryFastPath(move(gs), {}, frefs, core::lsp::Query::createSymbolQuery(sym));
}

bool LSPLoop::ensureInitialized(LSPMethod forMethod, const LSPMessage &msg,
                                const unique_ptr<core::GlobalState> &currentGs) const {
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

    if (enableTypecheckInfo) {
        vector<string> pathsTypechecked;
        for (auto &f : filesTypechecked) {
            pathsTypechecked.emplace_back(f.data(gs).path());
        }
        auto sorbetTypecheckInfo = make_unique<SorbetTypecheckRunInfo>(run.tookFastPath, move(pathsTypechecked));
        responses.push_back(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetTypecheckRunInfo, move(sorbetTypecheckInfo))));
    }

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
                    uri = fileRef2Uri(gs, file);
                }
            }

            vector<unique_ptr<Diagnostic>> diagnostics;
            {
                // diagnostics
                if (errorsAccumulated.find(file) != errorsAccumulated.end()) {
                    for (auto &e : errorsAccumulated[file]) {
                        auto range = loc2Range(gs, e->loc);
                        if (range == nullptr) {
                            continue;
                        }
                        auto diagnostic = make_unique<Diagnostic>(std::move(range), e->header);
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
                                    auto location = loc2Location(gs, errorLine.loc);
                                    if (location == nullptr) {
                                        continue;
                                    }
                                    relatedInformation.push_back(
                                        make_unique<DiagnosticRelatedInformation>(std::move(location), message));
                                }
                            }
                            // Add link to error documentation.
                            relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(
                                make_unique<Location>(
                                    absl::StrCat(opts.errorUrlBase, e->what.code),
                                    make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(0, 0))),
                                "Click for more information on this error."));
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

bool LSPLoop::shouldSendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) const {
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
