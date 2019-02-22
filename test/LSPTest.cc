#include "test/LSPTest.h"

#include <signal.h>

#include "main/lsp/json_types.h"
#include "main/realmain.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace sorbet::test {
using namespace std;

void LSPTest::SetUp() {
    test = GetParam();
    // All of this stuff is ignored by LSP, but we need it to construct ErrorQueue/GlobalState.
    // Cargo-culting from realmain.cc and other test runners.
    stderrColorSink = make_shared<spd::sinks::ansicolor_stderr_sink_mt>();
    auto logger = make_shared<spd::logger>("console", stderrColorSink);
    typeErrorsConsole = make_shared<spd::logger>("typeDiagnostics", stderrColorSink);
    typeErrorsConsole->set_pattern("%v");
    unique_ptr<core::GlobalState> gs =
        make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*typeErrorsConsole, *logger)));
    unique_ptr<KeyValueStore> kvstore;
    realmain::options::Options opts;
    realmain::createInitialGlobalState(gs, logger, opts, kvstore);
    // If we don't tell the errorQueue to ignore flushes, then we won't get diagnostic messages.
    gs->errorQueue->ignoreFlushes = true;

    lspWrapper = make_unique<LSPWrapper>(move(gs), move(opts), logger, fastpathDisabled);

    parseTestFile();
}

bool LSPTest::fastpathDisabled = false;

void LSPTest::parseTestFile() {
    for (auto &sourceFile : test.sourceFiles) {
        filenames.insert(test.folder + sourceFile);
    }

    assertions = RangeAssertion::parseAssertions(test.sourceFileContents);

    if (test.expectations.find("autogen") != test.expectations.end()) {
        // When autogen is enabled, skip DSL passes...
        lspWrapper->opts.skipDSLPasses = true;
        // Some autogen tests assume that some errors will occur from the resolver step, others assume the resolver
        // won't run.
        if (assertions.size() > 0) {
            // ...and stop after the resolver phase if there are errors
            lspWrapper->opts.stopAfterPhase = realmain::options::Phase::RESOLVER;
        } else {
            // ...and stop after the namer phase if there are no errors
            lspWrapper->opts.stopAfterPhase = realmain::options::Phase::NAMER;
        }
    }
}

} // namespace sorbet::test
