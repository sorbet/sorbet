#ifndef TEST_LSPTEST_H
#define TEST_LSPTEST_H

#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.

#include "main/lsp/wrapper.h"
#include "test/helpers/expectations.h"
#include "test/helpers/position_assertions.h"

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

/**
 * Parameterized GTest test pattern for LSP. Handles parsing the test file, initializing LSP,
 * feeding messages to it, and receiving messages from it.
 */
class LSPTest : public testing::TestWithParam<Expectations> {
private:
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
