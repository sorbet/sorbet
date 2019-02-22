#ifndef TEST_LSPTEST_H
#define TEST_LSPTEST_H

#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.

#include "spdlog/spdlog.h"
// has to come before the next one. This comment stops formatter from reordering them
#include "main/lsp/wrapper.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "test/expectations.h"
#include "test/position_assertions.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

/**
 * Parameterized GTest test pattern for LSP. Handles parsing the test file, initializing LSP,
 * feeding messages to it, and receiving messages from it.
 */
class LSPTest : public testing::TestWithParam<Expectations> {
private:
    // Note: LSPTest must keep these objects alive, as LSPLoop captures a reference to it.
    std::shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
    std::shared_ptr<spd::logger> typeErrorsConsole;

    /** Parses the test file and its assertions. */
    void parseTestFile();

    /** Starts up the LSP 'server'. */
    void startLSP();

protected:
    /** Parses test file and initializes LSP */
    void SetUp() override;

    void TearDown() override {}

public:
    /** The path to the test Ruby files on disk */
    UnorderedSet<std::string> filenames;
    std::unique_ptr<LSPWrapper> lspWrapper;

    /** All test assertions ordered by (filename, range, message). */
    std::vector<std::shared_ptr<RangeAssertion>> assertions;

    /** Test expectations. Stored here for convenience. */
    Expectations test;

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;
    static bool fastpathDisabled;
};
} // namespace sorbet::test
#endif // TEST_LSPTEST_H
