#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "test/helpers/MockFileSystem.h"
#include <cstddef>
#include <cstdint>
#include <memory>

auto console = spdlog::stdout_logger_mt("console");
auto typeErrors = spdlog::stdout_logger_mt("typeErrors");

sorbet::realmain::options::Options mkOpts() {
    const auto rootPath = "/tmp";
    sorbet::realmain::options::Options opts;
    opts.fs = std::make_shared<sorbet::test::MockFileSystem>(rootPath);
    return opts;
}

std::unique_ptr<sorbet::core::GlobalState> mkGlobalState(const sorbet::realmain::options::Options &opts) {
    std::unique_ptr<sorbet::core::GlobalState> gs = std::make_unique<sorbet::core::GlobalState>(
        (std::make_shared<sorbet::core::ErrorQueue>(*typeErrors, *console)));
    std::unique_ptr<sorbet::KeyValueStore> kvstore;
    sorbet::payload::createInitialGlobalState(gs, opts, kvstore);
    return gs;
}

extern "C" int LLVMFuzzerInitialize(const int *argc, const char ***argv) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const std::size_t size) {
    const auto opts = mkOpts();
    const auto gs = mkGlobalState(opts);
    return 0;
}
