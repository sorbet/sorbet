#include "core/NullFlusher.h"
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
const auto fileName = "file.rb";
const auto filePath = fmt::format("{}/{}", rootPath, fileName);
const auto fileUri = sorbet::test::filePathToUri(rootUri, fileName);

shared_ptr<sorbet::realmain::options::Options> mkOpts(string_view contents) {
    auto opts = make_shared<sorbet::realmain::options::Options>();
    opts->fs = make_shared<sorbet::test::MockFileSystem>(rootPath);
    opts->fs->writeFile(filePath, contents);
    opts->rawInputDirNames.emplace_back(rootPath);
    opts->inputFileNames.emplace_back(filePath);
    return opts;
}

unique_ptr<sorbet::core::GlobalState> mkGlobalState(const sorbet::realmain::options::Options &opts,
                                                    unique_ptr<sorbet::KeyValueStore> &kvStore) {
    auto gs = make_unique<sorbet::core::GlobalState>(
        (make_shared<sorbet::core::ErrorQueue>(*typeErrors, *console, make_shared<core::NullFlusher>())));
    sorbet::payload::createInitialGlobalState(gs, opts, kvStore);
    return gs;
}

unique_ptr<sorbet::realmain::lsp::SingleThreadedLSPWrapper> mkLSPWrapper(string_view contents) {
    unique_ptr<sorbet::KeyValueStore> kvStore;
    auto opts = mkOpts(contents);
    static const auto commonGs = mkGlobalState(*opts, kvStore);
    // TODO how to use opts and avoid another mkOpts()?
    auto lspWrapper = sorbet::realmain::lsp::SingleThreadedLSPWrapper::createWithGlobalState(
        commonGs->deepCopy(true), mkOpts(contents), console, true);
    lspWrapper->enableAllExperimentalFeatures();
    return lspWrapper;
}

extern "C" int LLVMFuzzerInitialize(const int *argc, const char ***argv) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const std::size_t size) {
    string contents((const char *)data, size);
    auto lspWrapper = mkLSPWrapper(contents);
    int nextId = 0;
    sorbet::test::initializeLSP(rootPath, rootUri, *lspWrapper, nextId);
    ENFORCE(lspWrapper
                ->getLSPResponsesFor(sorbet::test::LSPMessage(make_unique<sorbet::test::RequestMessage>(
                    "2.0", nextId++, sorbet::test::LSPMethod::TextDocumentDocumentSymbol,
                    make_unique<sorbet::test::DocumentSymbolParams>(
                        make_unique<sorbet::test::TextDocumentIdentifier>(fileUri)))))
                .size() == 1);
    return 0;
}
