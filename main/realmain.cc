#include "main/realmain.h"
#include "core/Errors.h"
#include "core/Files.h"
#include "core/Unfreeze.h"
#include "core/proto/proto.h"
#include "core/serialize/serialize.h"
#include "core/statsd/statsd.h"
#include "main/errorqueue/ConcurrentErrorQueue.h"
#include "main/lsp/lsp.h"
#include "main/pipeline/pipeline.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"
#include "spdlog/fmt/ostr.h"
#include "version/version.h"

#include "absl/strings/str_cat.h"

#include <algorithm> // find
#include <csignal>
#include <iostream>
#include <poll.h>

namespace spd = spdlog;

using namespace std;

namespace sorbet {
namespace realmain {
shared_ptr<spd::logger> logger;
int returnCode;

shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> make_stderr_color_sink() {
    auto color_sink = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    color_sink->set_color(spd::level::info, color_sink->white);
    color_sink->set_color(spd::level::debug, color_sink->magenta);
    return color_sink;
}

shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderr_color_sink = make_stderr_color_sink();

const string GLOBAL_STATE_KEY = "GlobalState";

void createInitialGlobalState(unique_ptr<core::GlobalState> &gs, const options::Options &options,
                              unique_ptr<KeyValueStore> &kvstore) {
    if (kvstore) {
        auto maybeGsBytes = kvstore->read(GLOBAL_STATE_KEY);
        if (maybeGsBytes) {
            Timer timeit(logger, "Read cached global state");
            core::serialize::Serializer::loadGlobalState(*gs, maybeGsBytes);
            for (unsigned int i = 1; i < gs->filesUsed(); i++) {
                core::FileRef fref(i);
                if (fref.data(*gs, true).sourceType == core::File::Type::Normal) {
                    gs = core::GlobalState::markFileAsTombStone(move(gs), fref);
                }
            }
            return;
        }
    }
    if (options.noStdlib) {
        gs->initEmpty();
        return;
    }

    const u1 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        gs->initEmpty();
        Timer timeit(logger, "Indexed payload");

        vector<core::FileRef> payloadFiles;
        {
            core::UnfreezeFileTable fileTableAccess(*gs);
            for (auto &p : rbi::all()) {
                auto file = gs->enterFile(p.first, p.second);
                file.data(*gs).sourceType = core::File::PayloadGeneration;
                payloadFiles.push_back(move(file));
            }
        }
        options::Options emptyOpts;
        emptyOpts.threads = 1;
        WorkerPool workers(emptyOpts.threads, logger);
        vector<string> empty;
        auto indexed = pipeline::index(gs, empty, payloadFiles, emptyOpts, workers, kvstore, logger);
        pipeline::resolve(*gs, move(indexed), emptyOpts, logger); // result is thrown away
    } else {
        Timer timeit(logger, "Read serialized payload");
        core::serialize::Serializer::loadGlobalState(*gs, nameTablePayload);
    }
}

/*
 * Workaround https://bugzilla.mindrot.org/show_bug.cgi?id=2863 ; We are
 * commonly run under ssh with a controlmaster, and we write exclusively to
 * STDERR in normal usage. If the client goes away, we can hang forever writing
 * to a full pipe buffer on stderr.
 *
 * Workaround by monitoring for STDOUT to go away and self-HUPing.
 */
void startHUPMonitor() {
    thread monitor([]() {
        struct pollfd pfd;
        pfd.fd = 1; // STDOUT
        pfd.events = 0;
        pfd.revents = 0;
        while (true) {
            int rv = poll(&pfd, 1, -1);
            if (rv <= 0) {
                continue;
            }
            if ((pfd.revents & (POLLHUP | POLLERR)) != 0) {
                // STDOUT has gone away; Exit via SIGHUP.
                kill(getpid(), SIGHUP);
            }
        }
    });
    monitor.detach();
}

int realmain(int argc, char *argv[]) {
    returnCode = 0;
    logger = spd::details::registry::instance().create("console", stderr_color_sink);
    logger->set_level(spd::level::trace); // pass through everything, let the sinks decide
    logger->set_pattern("%v");
    fatalLogger = logger;

    auto typeErrorsConsole = spd::details::registry::instance().create("typeErrors", stderr_color_sink);

    options::Options opts;
    options::readOptions(opts, argc, argv, logger);
    if (opts.stdoutHUPHack) {
        startHUPMonitor();
    }
    if (!opts.debugLogFile.empty()) {
        auto fileSink = make_shared<spd::sinks::simple_file_sink_mt>(opts.debugLogFile);
        fileSink->set_level(spd::level::debug);
        { // replace console & fatal loggers
            vector<spd::sink_ptr> sinks{stderr_color_sink, fileSink};
            auto combinedLogger = make_shared<spd::logger>("consoleAndFile", begin(sinks), end(sinks));
            combinedLogger->flush_on(spdlog::level::err);
            combinedLogger->set_level(spd::level::trace); // pass through everything, let the sinks decide

            spd::register_logger(combinedLogger);
            fatalLogger = combinedLogger;
            logger = combinedLogger;
        }
        { // replace type error logger
            vector<spd::sink_ptr> sinks{stderr_color_sink, fileSink};
            auto combinedLogger = make_shared<spd::logger>("typeErrorsAndFile", begin(sinks), end(sinks));
            spd::register_logger(combinedLogger);
            combinedLogger->set_level(spd::level::trace); // pass through everything, let the sinks decide
            typeErrorsConsole = combinedLogger;
        }
    }
    logger->set_pattern("%v");
    // Use a custom formatter so we don't get a default newline
    auto formatter = make_shared<spd::pattern_formatter>("%v", spd::pattern_time_type::local, "");
    typeErrorsConsole->set_formatter(formatter);

    switch (opts.logLevel) {
        case 0:
            stderr_color_sink->set_level(spd::level::info);
            break;
        case 1:
            stderr_color_sink->set_level(spd::level::debug);
            logger->set_pattern("[T%t][%Y-%m-%dT%T.%f] %v");
            logger->debug("Debug logging enabled");
            break;
        default:
            stderr_color_sink->set_level(spd::level::trace);
            logger->set_pattern("[T%t][%Y-%m-%dT%T.%f] %v");
            logger->trace("Trace logging enabled");
            break;
    }

    {
        string argsConcat(argv[0]);
        for (int i = 1; i < argc; i++) {
            string argString(argv[i]);
            argsConcat = argsConcat + " " + argString;
        }
        logger->debug("Running sorbet version {} with arguments: {}", Version::build_scm_revision, argsConcat);
    }
    WorkerPool workers(opts.threads, logger);

    unique_ptr<core::GlobalState> gs =
        make_unique<core::GlobalState>((make_shared<ConcurrentErrorQueue>(*typeErrorsConsole, *logger)));

    logger->trace("building initial global state");
    unique_ptr<KeyValueStore> kvstore;
    if (!opts.cacheDir.empty()) {
        kvstore = make_unique<KeyValueStore>(Version::build_scm_revision, opts.cacheDir);
    }
    createInitialGlobalState(gs, opts, kvstore);
    if (opts.silenceErrors) {
        gs->silenceErrors = true;
    }
    if (opts.autocorrect) {
        gs->autocorrect = true;
    }
    if (opts.reserveMemKiB > 0) {
        gs->reserveMemory(opts.reserveMemKiB);
    }
    logger->trace("done building initial global state");

    if (opts.runLSP) {
        gs->errorQueue->ignoreFlushes = true;
        lsp::LSPLoop loop(move(gs), opts, logger, workers);
        loop.runLSP();
        return 0;
    }
    Timer timeall(logger, "Done in");
    vector<core::FileRef> inputFiles;
    logger->trace("Files: ");
    {
        Timer timeit(logger, "reading files");
        core::UnfreezeFileTable fileTableAccess(*gs);
        if (!opts.inlineInput.empty()) {
            core::prodCounterAdd("types.input.bytes", opts.inlineInput.size());
            core::prodCounterInc("types.input.lines");
            core::prodCounterInc("types.input.files");
            auto file = gs->enterFile(string("-e"), opts.inlineInput + "\n");
            inputFiles.push_back(file);
            if (opts.forceMaxStrict < core::StrictLevel::Typed) {
                logger->error("`-e` is incompatible with `--typed=ruby`");
                return 1;
            }
            file.data(*gs).strict = core::StrictLevel::Strict;
        }
    }

    vector<unique_ptr<ast::Expression>> indexed;
    {
        Timer timeit(logger, "index");
        indexed = pipeline::index(gs, opts.inputFileNames, inputFiles, opts, workers, kvstore, logger);
    }

    if (kvstore && gs->wasModified() && !gs->hadCriticalError()) {
        Timer timeit(logger, "caching global state");
        kvstore->write(GLOBAL_STATE_KEY, core::serialize::Serializer::storePayloadAndNameTable(*gs));
        KeyValueStore::commit(move(kvstore));
    }

    indexed = pipeline::typecheck(gs, pipeline::resolve(*gs, move(indexed), opts, logger), opts, workers, logger);

    gs->errorQueue->flushErrors();

    if (opts.print.ErrorFiles) {
        for (auto &tree : indexed) {
            auto f = tree->loc.file;
            if (f.data(*gs).hadErrors()) {
                cout << f.data(*gs).path() << "\n";
            }
        }
    } else if (!opts.noErrorCount) {
        gs->errorQueue->flushErrorCount();
    }
    if (opts.autocorrect) {
        gs->errorQueue->flushAutocorrects(*gs);
    }
    logger->trace("sorbet done");

    if (opts.suggestTyped) {
        for (auto &tree : indexed) {
            auto f = tree->loc.file;
            if (!f.data(*gs).hadErrors() && f.data(*gs).sigil == core::StrictLevel::Stripe) {
                core::counterInc("types.input.files.suggest_typed");
                logger->error("You could add `# typed: true` to: `{}`", f.data(*gs).path());
            }
        }
    }

    if (!opts.storeState.empty()) {
        gs->markAsPayload();
        FileOps::write(opts.storeState.c_str(), core::serialize::Serializer::store(*gs));
    }

    if (!opts.someCounters.empty()) {
        if (opts.enableCounters) {
            logger->error("Don't pass both --counters and --counter");
            return 1;
        }
        logger->warn("" + core::getCounterStatistics(opts.someCounters));
    }

    if (opts.enableCounters) {
        logger->warn("" + core::getCounterStatistics(core::Counters::ALL_COUNTERS));
    } else {
        logger->debug("" + core::getCounterStatistics(core::Counters::ALL_COUNTERS));
    }

    auto counters = core::getAndClearThreadCounters();

    if (!opts.statsdHost.empty()) {
        core::StatsD::submitCounters(counters, opts.statsdHost, opts.statsdPort, opts.statsdPrefix + ".counters");
    }

    if (!opts.metricsFile.empty()) {
        auto metrics = core::Proto::toProto(counters, opts.metricsPrefix);
        string status;
        if (gs->hadCriticalError()) {
            status = "Error";
        } else if (returnCode != 0) {
            status = "Failure";
        } else {
            status = "Success";
        }

        if (opts.suggestTyped) {
            for (auto &tree : indexed) {
                auto f = tree->loc.file;
                if (f.data(*gs).sigil == core::StrictLevel::Stripe) {
                    auto *metric = metrics.add_metrics();
                    metric->set_name(absl::StrCat(opts.metricsPrefix, ".suggest.", f.data(*gs).path()));
                    metric->set_value(!f.data(*gs).hadErrors());
                }
            }
        }

        metrics.set_repo(opts.metricsRepo);
        metrics.set_branch(opts.metricsBranch);
        metrics.set_sha(opts.metricsSha);
        metrics.set_status(status);

        auto json = core::Proto::toJSON(metrics);
        FileOps::write(opts.metricsFile, json);
    }
    if (gs->hadCriticalError()) {
        returnCode = 10;
    } else if (returnCode == 0 && gs->totalErrors() > 0 && !opts.supressNonCriticalErrors) {
        returnCode = 1;
    }

    // Let it go: leak memory so that we don't need to call destructors
    for (auto &e : indexed) {
        e.release();
    }
    gs.release();

    // je_malloc_stats_print(nullptr, nullptr, nullptr); // uncomment this to print jemalloc statistics

    return returnCode;
}

} // namespace realmain
} // namespace sorbet
