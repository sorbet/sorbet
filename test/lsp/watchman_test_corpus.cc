#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test::lsp {
using namespace std;
using namespace sorbet::realmain::lsp;

// Adds a file to the file system with an error, and asserts that Sorbet returns an error.
TEST_CASE_FIXTURE(ProtocolTest, "UpdateFileOnFileSystem") {
    assertDiagnostics(initializeLSP(), {});
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});
}

// Creates an empty file and deletes it.
TEST_CASE_FIXTURE(ProtocolTest, "CreateAndDeleteEmptyFile") {
    assertDiagnostics(initializeLSP(), {});
    writeFilesToFS({{"foo.rb", ""}});
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});

    deleteFileFromFS("foo.rb");
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
}

// Adds a file with an error, and then deletes that file. Asserts that Sorbet no longer complains about the file.
TEST_CASE_FIXTURE(ProtocolTest, "DeleteFileWithErrors") {
    assertDiagnostics(initializeLSP(), {});
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    deleteFileFromFS("foo.rb");
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
}

// Informs Sorbet about a file update for a file it does not know about and is deleted on disk. Should be a no-op.
TEST_CASE_FIXTURE(ProtocolTest, "DeleteFileUnknownToSorbet") {
    assertDiagnostics(initializeLSP(), {});
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
}

// Updates a file, opens it in editor (but it's empty), closes file without saving to disk.
TEST_CASE_FIXTURE(ProtocolTest, "IgnoresLSPFileUpdatesWhileFileIsOpen") {
    assertDiagnostics(initializeLSP(), {});

    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    // Diagnostics should update now that we've opened the file in editor and it's empty.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});
    // File on disk is still buggy, but Sorbet should ignore disk updates while file is open in editor.
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
    // Sorbet should pick up buggy disk version after user closes file.
    assertDiagnostics(send(*closeFile("foo.rb")), {d});
}

// Ensures that Sorbet correctly remembers that a file is not open in the editor when it combines a file close event
// with another type of file update.
TEST_CASE_FIXTURE(ProtocolTest, "CorrectlyUpdatesFileOpenStatusWhenClosedCombinedWithOtherUpdates") {
    assertDiagnostics(initializeLSP(), {});

    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    // Diagnostics should update now that we've opened the file in editor and it's empty.
    assertDiagnostics(send(*openFile("foo.rb", "")), {});

    // Close + add another update in one atomic action.
    vector<unique_ptr<LSPMessage>> toSend;
    toSend.push_back(closeFile("foo.rb"));
    toSend.push_back(watchmanFileUpdate({"foo.rb"}));
    assertDiagnostics(send(move(toSend)), {d});

    // Ensure that Sorbet knows file is closed.
    assertDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});
}

// If file closes and is not on disk, Sorbet clears diagnostics.
TEST_CASE_FIXTURE(ProtocolTest, "HandlesClosedAndDeletedFile") {
    assertDiagnostics(initializeLSP(), {});
    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    assertDiagnostics(
        send(*openFile("foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n")), {d});
    assertDiagnostics(send(*closeFile("foo.rb")), {});
}

// Sorbet merges all pending watchman updates into a single update.
TEST_CASE_FIXTURE(ProtocolTest, "MergesMultipleWatchmanUpdates") {
    assertDiagnostics(initializeLSP(), {});
    vector<unique_ptr<LSPMessage>> requests;
    // If processed serially, these would cause slow path runs (new files).
    requests.push_back(watchmanFileUpdate({"foo.rb"}));
    requests.push_back(watchmanFileUpdate({"bar.rb", "foo.rb"}));
    requests.push_back(watchmanFileUpdate({"baz.rb"}));
    // If processed serially, these would cause fast path runs.
    requests.push_back(watchmanFileUpdate({"foo.rb"}));
    requests.push_back(watchmanFileUpdate({"bar.rb", "baz.rb"}));

    string buggyFileContents = "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n";
    writeFilesToFS({{"foo.rb", buggyFileContents}, {"bar.rb", buggyFileContents}, {"baz.rb", buggyFileContents}});
    assertDiagnostics(send(move(requests)), {
                                                {"foo.rb", 3, "Expected `Integer`"},
                                                {"bar.rb", 3, "Expected `Integer`"},
                                                {"baz.rb", 3, "Expected `Integer`"},
                                            });

    // getTypecheckCount tracks the number of times typechecking has run on the same clone from LSPLoop's
    // initialGS. It's reset to 1 after each slow path run, and incremented after every fast path.
    // We expect the merged case to run 1 slow path (where typecheck count would be 1), and the unmerged case to run
    // 3 slow paths and 2 fast paths (where typecheck count would be 3).
    INFO(fmt::format("Expected Sorbet to apply multiple Watchman updates in one typechecking run, but Sorbet ran "
                     "typechecking {} times.",
                     lspWrapper->getTypecheckCount()));
    CHECK_EQ(lspWrapper->getTypecheckCount(), 1);
}

} // namespace sorbet::test::lsp
