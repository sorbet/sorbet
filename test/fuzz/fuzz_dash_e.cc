#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "spdlog/sinks/stdout_sinks.h"
#include <cstddef>
#include <cstdint>

using namespace std;
using namespace sorbet;
namespace spd = spdlog;

auto logger = spdlog::stdout_logger_mt("console");
auto typeErrorsConsole = spdlog::stdout_logger_mt("typeErrorsConsole");

const realmain::options::Options opts = [] {
    realmain::options::Options opts;
    // you can set up options here
    return opts;
}();

unique_ptr<KeyValueStore> kvstore;

bool stressIncrementalResolver = false;

unique_ptr<core::GlobalState> buildInitialGlobalState() {
    typeErrorsConsole->set_level(spd::level::critical);

    unique_ptr<core::GlobalState> gs =
        make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));

    logger->trace("Doing on-start initialization");

    payload::createInitialGlobalState(gs, logger, opts, kvstore);
    return gs;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    logger->set_level(spd::level::critical);
    fatalLogger = logger;
    // Huh, I wish we could use cxxopts, but libfuzzer & cxxopts choke on each other argument formats
    // thus we do it manually
    for (int i = 0; i < *argc; i++) {
        if (string((*argv)[i]) == "--stress-incremental-resolver") {
            stressIncrementalResolver = true;
        }
    }

    if (stressIncrementalResolver) {
        logger->critical("Enabling incremental resolver");
    }
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static const unique_ptr<core::GlobalState> commonGs = buildInitialGlobalState();
    static WorkerPool workers(0, logger);
    commonGs->trace("starting run");
    unique_ptr<core::GlobalState> gs;
    { gs = commonGs->deepCopy(true); }
    string inputData((const char *)data, size);
    vector<ast::ParsedFile> indexed;
    vector<core::FileRef> inputFiles;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto file = gs->enterFile(string("fuzz.rb"), inputData);
        inputFiles.emplace_back(file);
        file.data(*gs).strictLevel = core::StrictLevel::Typed;
    }

    indexed = realmain::pipeline::index(gs, {}, inputFiles, opts, workers, kvstore, logger);
    indexed = realmain::pipeline::resolve(*gs, move(indexed), opts, logger);
    if (stressIncrementalResolver) {
        for (auto &f : indexed) {
            auto reIndexed = realmain::pipeline::indexOne(opts, *gs, f.file, kvstore, logger);
            vector<ast::ParsedFile> toBeReResolved;
            toBeReResolved.emplace_back(move(reIndexed));
            auto reresolved = realmain::pipeline::incrementalResolve(*gs, move(toBeReResolved), opts, logger);
            ENFORCE(reresolved.size() == 1);
            f = move(reresolved[0]);
        }
    }
    indexed = realmain::pipeline::typecheck(gs, move(indexed), opts, workers, logger);
    return 0;
}
