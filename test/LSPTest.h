#ifndef TEST_LSPTEST_H
#define TEST_LSPTEST_H

#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.

#include "spdlog/spdlog.h"
// has to come before the next one. This comment stops formatter from reordering them
#include "spdlog/sinks/stdout_color_sinks.h"
#include "test/expectations.h"
#include "test/position_assertions.h"

#include <sstream>

namespace sorbet::test {
using namespace sorbet::realmain::lsp;

/**
 * Parameterized GTest test pattern for LSP. Handles parsing the test file, initializing LSP,
 * feeding messages to it, and receiving messages from it.
 */
class LSPTest : public testing::TestWithParam<Expectations> {
private:
    /** The LSP 'server', which runs in the same thread as the tests. */
    std::unique_ptr<LSPLoop> lspLoop;

    /** The global state of type checking, as calculated by LSP. */
    std::unique_ptr<core::GlobalState> gs;

    /**
     * Required objects that Sorbet assumes we 'own'. Not having these here would result in memory errors, as Sorbet
     * captures references to them. Normally these are stack allocated, but we cannot do that with gtests which
     * implicitly call `SetUp()`.
     */
    std::shared_ptr<spd::logger> logger;
    std::shared_ptr<spd::logger> typeErrorsConsole;
    realmain::options::Options opts;
    std::shared_ptr<spd::sinks::ansicolor_stderr_sink_mt> stderrColorSink;
    std::unique_ptr<WorkerPool> workers;

    /** The output stream used by LSP. Lets us drain all response messages after sending messages to LSP. */
    std::stringstream lspOstream;

    /** Parses the given LSP message string into a NotificationMessage or ResponseMessage. If parsing fails, adds an
     * error to the currently running test. */
    std::optional<std::unique_ptr<JSONBaseType>> parseLSPResponse(std::string raw);

    /** Sends the given string directly to Sorbet's LSP component, and returns any response messages. */
    std::vector<std::unique_ptr<JSONBaseType>> getLSPResponsesForRaw(std::string json);

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

    /** All test assertions ordered by (filename, range, message). */
    std::vector<std::shared_ptr<RangeAssertion>> assertions;

    /** If true, then LSPLoop is initialized and is ready to receive requests. */
    bool initialized = false;

    /** Memory allocator for rapidjson objects. */
    rapidjson::MemoryPoolAllocator<> alloc;

    /** Test expectations. Stored here for convenience. */
    Expectations test;

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;

    /**
     * Send a message to LSP, and returns any responses.
     */
    std::vector<std::unique_ptr<JSONBaseType>> getLSPResponsesFor(const std::unique_ptr<JSONBaseType> &message);
    static bool fastpathDisabled;
};
} // namespace sorbet::test
#endif // TEST_LSPTEST_H
