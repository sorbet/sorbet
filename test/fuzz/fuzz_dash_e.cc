#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "spdlog/sinks/stdout_sinks.h"
#include <cstddef>
#include <cstdint>

using namespace std;
using namespace sorbet;

auto logger = spdlog::stdout_logger_mt("console");
auto typeErrorsConsole = spdlog::stdout_logger_mt("typeErrorsConsole");

realmain::options::Options createDefaultOptions(bool stressIncrementalResolver) {
    realmain::options::Options opts;
    opts.stressIncrementalResolver = stressIncrementalResolver;
    // you can set up options here
    return opts;
};

unique_ptr<const realmain::options::Options> opts =
    make_unique<const realmain::options::Options>(createDefaultOptions(false));

unique_ptr<const OwnedKeyValueStore> kvstore;

unique_ptr<core::GlobalState> buildInitialGlobalState() {
    typeErrorsConsole->set_level(spdlog::level::critical);

    unique_ptr<core::GlobalState> gs =
        make_unique<core::GlobalState>(make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger));

    logger->trace("Doing on-start initialization");

    payload::createInitialGlobalState(gs, *opts, kvstore);
    return gs;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    logger->set_level(spdlog::level::critical);
    fatalLogger = logger;
    // Huh, I wish we could use cxxopts, but libfuzzer & cxxopts choke on each other argument formats
    // thus we do it manually
    for (int i = 0; i < *argc; i++) {
        if (string((*argv)[i]) == "--stress-incremental-resolver") {
            opts = make_unique<const realmain::options::Options>(createDefaultOptions(true));
        }
    }

    if (opts->stressIncrementalResolver) {
        logger->critical("Enabling incremental resolver");
    }
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static const unique_ptr<core::GlobalState> commonGs = buildInitialGlobalState();
    static unique_ptr<WorkerPool> workers = WorkerPool::create(0, *logger);
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
        file.data(*gs).strictLevel = core::StrictLevel::True;
    }

    indexed = realmain::pipeline::index(*gs, inputFiles, *opts, *workers, kvstore);
    indexed = move(realmain::pipeline::resolve(gs, move(indexed), *opts, *workers).result());
    realmain::pipeline::typecheck(gs, move(indexed), *opts, *workers);
    return 0;
}
