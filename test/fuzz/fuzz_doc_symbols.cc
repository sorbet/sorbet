#include "core/Unfreeze.h"
#include "main/lsp/wrapper.h"
#include "main/pipeline/pipeline.h"
#include "payload/payload.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "test/helpers/MockFileSystem.h"
#include "test/helpers/lsp.h"
#include <cstddef>
#include <cstdint>
#include <memory>

const auto console = spdlog::stdout_logger_mt("console");
const auto typeErrors = spdlog::stdout_logger_mt("typeErrors");
const auto rootPath = "/tmp";
const auto rootUri = fmt::format("file://{}", rootPath);

sorbet::realmain::options::Options mkOpts() {
    sorbet::realmain::options::Options opts;
    opts.fs = std::make_shared<sorbet::test::MockFileSystem>(rootPath);
    return opts;
}

std::unique_ptr<sorbet::core::GlobalState> mkGlobalState(const sorbet::realmain::options::Options &opts,
                                                         std::unique_ptr<sorbet::KeyValueStore> &kvstore) {
    std::unique_ptr<sorbet::core::GlobalState> gs = std::make_unique<sorbet::core::GlobalState>(
        (std::make_shared<sorbet::core::ErrorQueue>(*typeErrors, *console)));
    sorbet::payload::createInitialGlobalState(gs, opts, kvstore);
    return gs;
}

extern "C" int LLVMFuzzerInitialize(const int *argc, const char ***argv) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const std::size_t size) {
    std::unique_ptr<sorbet::KeyValueStore> kvstore;
    auto opts = mkOpts();
    static const auto commonGs = mkGlobalState(opts, kvstore);

    // TODO why?
    std::unique_ptr<sorbet::core::GlobalState> gs;
    { gs = commonGs->deepCopy(true); }

    // TODO move the gs?
    // TODO how to use opts and avoid another mkOpts()?
    auto lspWrapper = sorbet::realmain::lsp::LSPWrapper(std::move(gs), mkOpts(), console, false);

    int nextId = 0;
    // TODO ignore responses?
    sorbet::test::initializeLSP(rootPath, rootUri, lspWrapper, nextId);

    std::vector<sorbet::core::FileRef> inputFiles;
    std::string inputData((const char *)data, size);
    {
        sorbet::core::UnfreezeFileTable fileTableAccess(*gs);
        auto file = gs->enterFile(std::string("fuzz.rb"), inputData);
        inputFiles.emplace_back(file);
        file.data(*gs).strictLevel = sorbet::core::StrictLevel::True;
    }

    return 0;
}
