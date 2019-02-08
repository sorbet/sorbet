#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"
#include "main/realmain.h"
#include "spdlog/sinks/stdout_sinks.h"
#include <cstddef>
#include <cstdint>

using namespace std;
using namespace sorbet;
namespace spd = spdlog;

const realmain::options::Options opts = [] {
    realmain::options::Options opts;
    // you can set up options here
    return opts;
}();

unique_ptr<KeyValueStore> kvstore;

unique_ptr<core::GlobalState> buildInitialGlobalState() {
    realmain::logger = spdlog::stdout_logger_mt("console");
    realmain::logger->set_level(spd::level::critical);
    fatalLogger = realmain::logger;

    auto typeErrorsConsole = spdlog::stdout_logger_mt("typeErrorsConsole");
    typeErrorsConsole->set_level(spd::level::critical);

    unique_ptr<core::GlobalState> gs =
        make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *realmain::logger)));

    realmain::logger->trace("Doing on-start initialization");

    realmain::createInitialGlobalState(gs, realmain::logger, opts, kvstore);
    return gs;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static const unique_ptr<core::GlobalState> commonGs = buildInitialGlobalState();
    static WorkerPool workers(0, realmain::logger);
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
        file.data(*gs).strict = core::StrictLevel::Typed;
    }

    indexed = realmain::pipeline::index(gs, {}, inputFiles, opts, workers, kvstore, realmain::logger);
    indexed = realmain::pipeline::resolve(*gs, move(indexed), opts, realmain::logger);
    indexed = realmain::pipeline::typecheck(gs, move(indexed), opts, workers, realmain::logger);
    return 0;
}
