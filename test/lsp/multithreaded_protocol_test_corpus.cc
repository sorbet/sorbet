#include "gtest/gtest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "absl/strings/match.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

namespace {
void assertTypecheckRunInfo(const unique_ptr<LSPMessage> &msg, SorbetTypecheckRunStatus status, bool fastPath) {
    ASSERT_NE(nullptr, msg);
    ASSERT_EQ(true, msg->isNotification());
    ASSERT_EQ(LSPMethod::SorbetTypecheckRunInfo, msg->method());

    auto &info = get<unique_ptr<SorbetTypecheckRunInfo>>(msg->asNotification().params);
    ASSERT_EQ(status, info->status);
    ASSERT_EQ(fastPath, info->isFastPath);
}

/**void assertTypecheckRunInfo(const vector<unique_ptr<LSPMessage>> messages, SorbetTypecheckRunStatus status,
                            bool tookFastPath) {
    for (const auto &msg : messages) {
        if (msg->isNotification() && msg->method() == LSPMethod::SorbetTypecheckRunInfo) {
            assertTypecheckRunInfo(msg, status, tookFastPath);
            return;
        }
    }
    ADD_FAILURE() << "Expected a TypecheckInfo response, but received none!";
}*/
} // namespace

TEST_P(ProtocolTest, MultithreadedWrapperWorks) {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo2\n  def branch\n    2 + \"dog\"\n  end\nend\n", 2));
    requests.push_back(
        changeFile("yolo1.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"bear\"\n  end\nend\n", 3));
    assertDiagnostics(send(move(requests)), {{"yolo1.rb", 3, "bear"}});
}

TEST_P(ProtocolTest, CancelingSlowPathUpdatesDiagnostics) {
    auto opts = make_unique<SorbetInitializationOptions>();
    opts->enableTypecheckInfo = true;
    initializeLSP(true, move(opts));
    // New file; should take slow path.
    send(*openFile("foo.rb", "#typed: true\nclass Foo\nend"));

    // Send an update that should take slow path, and that will wait for a preemption to happen.
    // The following happens asynchronously.
    sendAsync(*changeFile("foo.rb", "#typed: true\nclass Foo\ndef me\nend", 2, true /* expected cancelation */));

    // Wait for update to start.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::started, false));

    // Send an update that should take fast path (corrects syntax error).
    sendAsync(*changeFile("foo.rb", "#typed: true\nclass Foo\nend", 3));

    // Ensure that cancelation occurs.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::canceled, false));

    // Fast path should begin.
    ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(readAsync(), SorbetTypecheckRunStatus::started, true));

    // Keep reading until fast path ends.
    vector<unique_ptr<LSPMessage>> diagnostics;
    while (true) {
        auto msg = readAsync();
        ASSERT_NE(nullptr, msg);
        if (msg->isNotification() && msg->method() == LSPMethod::SorbetTypecheckRunInfo) {
            ASSERT_NO_FATAL_FAILURE(assertTypecheckRunInfo(msg, SorbetTypecheckRunStatus::ended, true));
            break;
        }
        diagnostics.push_back(move(msg));
    }
    assertDiagnostics(move(diagnostics), {});
}

// Run these tests in multi-threaded mode.
INSTANTIATE_TEST_SUITE_P(MultithreadedProtocolTests, ProtocolTest, testing::Values(true));

} // namespace sorbet::test::lsp
