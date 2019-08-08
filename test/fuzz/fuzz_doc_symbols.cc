#include "main/lsp/wrapper.h"
#include "payload/payload.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "test/helpers/MockFileSystem.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"
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
    opts.lspDocumentSymbolEnabled = true;
    opts.rawInputDirNames.emplace_back(rootPath);
    return opts;
}

std::unique_ptr<sorbet::core::GlobalState> mkGlobalState(const sorbet::realmain::options::Options &opts,
                                                         std::unique_ptr<sorbet::KeyValueStore> &kvstore) {
    auto gs = std::make_unique<sorbet::core::GlobalState>(
        (std::make_shared<sorbet::core::ErrorQueue>(*typeErrors, *console)));
    sorbet::payload::createInitialGlobalState(gs, opts, kvstore);
    gs->errorQueue->ignoreFlushes = true;
    return gs;
}

sorbet::realmain::lsp::LSPWrapper mkLSPWrapper() {
    std::unique_ptr<sorbet::KeyValueStore> kvstore;
    auto opts = mkOpts();
    static const auto commonGs = mkGlobalState(opts, kvstore);
    // TODO how to use opts and avoid another mkOpts()?
    auto lspWrapper = sorbet::realmain::lsp::LSPWrapper(commonGs->deepCopy(true), mkOpts(), console, true);
    lspWrapper.enableAllExperimentalFeatures();
    return lspWrapper;
}

extern "C" int LLVMFuzzerInitialize(const int *argc, const char ***argv) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const std::size_t size) {
    auto lspWrapper = mkLSPWrapper();
    int nextId = 0;
    sorbet::test::initializeLSP(rootPath, rootUri, lspWrapper, nextId);
    std::string contents((const char *)data, size);
    auto fileUri = sorbet::test::filePathToUri(rootUri, "file.rb");
    lspWrapper.getLSPResponsesFor(sorbet::test::LSPMessage(std::make_unique<sorbet::test::NotificationMessage>(
        "2.0", sorbet::test::LSPMethod::TextDocumentDidOpen,
        std::make_unique<sorbet::test::DidOpenTextDocumentParams>(
            std::make_unique<sorbet::test::TextDocumentItem>(fileUri, "ruby", 1, contents)))));
    lspWrapper.getLSPResponsesFor(sorbet::test::LSPMessage(std::make_unique<sorbet::test::RequestMessage>(
        "2.0", nextId++, sorbet::test::LSPMethod::TextDocumentDocumentSymbol,
        std::make_unique<sorbet::test::DocumentSymbolParams>(
            std::make_unique<sorbet::test::TextDocumentIdentifier>(fileUri)))));
    return 0;
}
