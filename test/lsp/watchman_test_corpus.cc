#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "ProtocolTest.h"
#include "common/common.h"
#include "test/helpers/lsp.h"

using namespace std;

namespace sorbet::test::lsp {
using namespace sorbet::realmain::lsp;

// Adds a file to the file system with an error, and asserts that Sorbet returns an error.
TEST_CASE_FIXTURE(ProtocolTest, "UpdateFileOnFileSystem") {
    assertErrorDiagnostics(initializeLSP(), {});
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});
}

// Creates an empty file and deletes it.
TEST_CASE_FIXTURE(ProtocolTest, "CreateAndDeleteEmptyFile") {
    assertErrorDiagnostics(initializeLSP(), {});
    writeFilesToFS({{"foo.rb", ""}});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});

    deleteFileFromFS("foo.rb");
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
}

// Adds a file with an error, and then deletes that file. Asserts that Sorbet no longer complains about the file.
TEST_CASE_FIXTURE(ProtocolTest, "DeleteFileWithErrors") {
    assertErrorDiagnostics(initializeLSP(), {});
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    deleteFileFromFS("foo.rb");
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
}

// Informs Sorbet about a file update for a file it does not know about and is deleted on disk. Should be a no-op.
TEST_CASE_FIXTURE(ProtocolTest, "DeleteFileUnknownToSorbet") {
    assertErrorDiagnostics(initializeLSP(), {});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
}

// Updates a file, opens it in editor (but it's empty), closes file without saving to disk.
TEST_CASE_FIXTURE(ProtocolTest, "IgnoresLSPFileUpdatesWhileFileIsOpen") {
    assertErrorDiagnostics(initializeLSP(), {});

    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    // Diagnostics should update now that we've opened the file in editor and it's empty.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});
    // File on disk is still buggy, but Sorbet should ignore disk updates while file is open in editor.
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});
    // Sorbet should pick up buggy disk version after user closes file.
    assertErrorDiagnostics(send(*closeFile("foo.rb")), {d});
}

// Ensures that Sorbet correctly remembers that a file is not open in the editor when it combines a file close event
// with another type of file update.
TEST_CASE_FIXTURE(ProtocolTest, "CorrectlyUpdatesFileOpenStatusWhenClosedCombinedWithOtherUpdates") {
    assertErrorDiagnostics(initializeLSP(), {});

    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    // Diagnostics should update now that we've opened the file in editor and it's empty.
    assertErrorDiagnostics(send(*openFile("foo.rb", "")), {});

    // Close + add another update in one atomic action.
    vector<unique_ptr<LSPMessage>> toSend;
    toSend.push_back(closeFile("foo.rb"));
    toSend.push_back(watchmanFileUpdate({"foo.rb"}));
    assertErrorDiagnostics(send(move(toSend)), {d});

    // Ensure that Sorbet knows file is closed.
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});
}

// If file closes and is not on disk, Sorbet clears diagnostics.
TEST_CASE_FIXTURE(ProtocolTest, "HandlesClosedAndDeletedFile") {
    assertErrorDiagnostics(initializeLSP(), {});
    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    assertErrorDiagnostics(
        send(*openFile("foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n")), {d});
    assertErrorDiagnostics(send(*closeFile("foo.rb")), {});
}

// Sorbet merges all pending watchman updates into a single update.
TEST_CASE_FIXTURE(ProtocolTest, "MergesMultipleWatchmanUpdates") {
    assertErrorDiagnostics(initializeLSP(), {});
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

    // Clear counters
    getCounters();

    assertErrorDiagnostics(send(move(requests)), {
                                                     {"foo.rb", 3, "Expected `Integer`"},
                                                     {"bar.rb", 3, "Expected `Integer`"},
                                                     {"baz.rb", 3, "Expected `Integer`"},
                                                 });

    auto counters = getCounters();

    INFO(fmt::format("Expected Sorbet to apply multiple Watchman updates in one typechecking run, but Sorbet ran "
                     "typechecking {} times.",
                     counters.getCategoryCounter("lsp.updates", "slowpath")));
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath_canceled"), 0);
}

// A fresh instance means Watchman may have dropped changes. Sorbet reads a file only when told to, and its slow path
// re-indexes out of the file table, so a dropped change would sit there for the life of the process.
TEST_CASE_FIXTURE(ProtocolTest, "ResyncsFilesChangedWhileWatchmanWasNotLooking") {
    assertErrorDiagnostics(initializeLSP(), {});

    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + 2\n  end\nend\n"}});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {});

    // Break the file without telling Sorbet, standing in for a change Watchman never delivered.
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});

    // A resync has to take the slow path; see LSPFileUpdates::resyncedAllFiles.
    getCounters();
    assertErrorDiagnostics(send(*watchmanFreshInstance()), {{"foo.rb", 3, "Expected `Integer`"}});

    auto counters = getCounters();
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "slowpath"), 1);
    CHECK_EQ(counters.getCategoryCounter("lsp.updates", "fastpath"), 0);
    CHECK_EQ(counters.getCategoryCounter("lsp.slow_path_reason", "resynced_all_files"), 1);
}

// A file created while Watchman was not looking is on disk but not in the file table, so only the walk turns it up.
TEST_CASE_FIXTURE(ProtocolTest, "ResyncsFilesCreatedWhileWatchmanWasNotLooking") {
    assertErrorDiagnostics(initializeLSP(), {});

    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertErrorDiagnostics(send(*watchmanFreshInstance()), {{"foo.rb", 3, "Expected `Integer`"}});
}

// A deleted one is the mirror image: in the file table but not on disk, so only the file table turns it up.
TEST_CASE_FIXTURE(ProtocolTest, "ResyncsFilesDeletedWhileWatchmanWasNotLooking") {
    assertErrorDiagnostics(initializeLSP(), {});

    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {{"foo.rb", 3, "Expected `Integer`"}});

    deleteFileFromFS("foo.rb");
    assertErrorDiagnostics(send(*watchmanFreshInstance()), {});
}

// The editor's copy of an open file supersedes what is on disk, so a resync must not read over it.
TEST_CASE_FIXTURE(ProtocolTest, "FreshInstanceLeavesFilesOpenInEditorAlone") {
    assertErrorDiagnostics(initializeLSP(), {});

    ExpectedDiagnostic d = {"foo.rb", 3, "Expected `Integer`"};
    writeFilesToFS({{"foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + \"stuff\"\n  end\nend\n"}});
    assertErrorDiagnostics(send(*watchmanFileUpdate({"foo.rb"})), {d});

    assertErrorDiagnostics(
        send(*openFile("foo.rb", "# typed: true\nclass Foo1\n  def branch\n    1 + 2\n  end\nend\n")), {});
    assertErrorDiagnostics(send(*watchmanFreshInstance()), {});

    // Sorbet goes back to the version on disk once the editor gives the file up.
    assertErrorDiagnostics(send(*closeFile("foo.rb")), {d});
}

TEST_CASE_FIXTURE(ProtocolTest, "ZeroingOutPackageFiles") {
    auto opts = make_shared<realmain::options::Options>();
    opts->cacheSensitiveOptions.sorbetPackages = true;
    this->resetState(std::move(opts));

    writeFilesToFS({
        {"__package.rb", "# typed: strict\n"
                         "# frozen_string_literal: true\n"
                         "class Project < PackageSpec\n"
                         "  export Project::Bar\n"
                         "end\n"},

        {"impl.rb", "# typed: strict\n"
                    "# frozen_string_literal: true\n"
                    "module Project\n"
                    "  class Bar\n"
                    "  end\n"
                    "end\n"},

        {"b/__package.rb", "# typed: strict\n"
                           "# frozen_string_literal: frue\n"
                           "class Project::B < PackageSpec\n"
                           "  export Project::B::Foo\n"
                           "  import Project::C\n"
                           "end\n"},

        {"b/impl.rb", "# typed: strict\n"
                      "# frozen_string_literal: true\n"
                      "module Project\n"
                      "  class B::Foo\n"
                      "    extend T::Sig\n"
                      "    sig { returns(Project::C::Bar) }\n"
                      "    def test\n"
                      "      Project::C::Bar.new\n"
                      "    end\n"
                      "  end\n"
                      "end\n"},

        {"c/__package.rb", "# typed: strict\n"
                           "# frozen_string_literal: true\n"
                           "class Project::C < PackageSpec\n"
                           "  export Project::C::Bar\n"
                           "end\n"},

        {"c/impl.rb", "# typed: strict\n"
                      "# frozen_string_literal: true\n"
                      "module Project\n"
                      "  class C::Bar\n"
                      "  end\n"
                      "end\n"},
    });

    // It's important that these files are present during initialization, to force any potential persistence issues
    // when the typechecker thread copies its GlobalState over to the indexer.
    this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/__package.rb", this->rootPath));
    this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/b/__package.rb", this->rootPath));
    this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/b/impl.rb", this->rootPath));
    this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/c/__package.rb", this->rootPath));
    this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/c/impl.rb", this->rootPath));
    this->lspWrapper->opts->inputFileNames.emplace_back(fmt::format("{}/impl.rb", this->rootPath));

    assertErrorDiagnostics(initializeLSP(), {});

    // Overwrite the contents of c/__package.rb, and trigger a slow path. This should only result in errors about the
    // new structure of the project, but if the packageDB is accidentally persisted through the GlobalState copy that's
    // given to the indexer, we'll see ENFORCE failures here when we try to use stale information from the original
    // packageDB to generate autocorrects.
    writeFilesToFS({{"c/__package.rb", "\n"}});
    vector<unique_ptr<LSPMessage>> requests;
    requests.push_back(watchmanFileUpdate({"c/__package.rb", "c/impl.rb"}));
    assertErrorDiagnostics(send(move(requests)), {
                                                     {"b/__package.rb", 4, "Unable to resolve constant"},
                                                     {"b/impl.rb", 5, "resolves but its package is not imported"},
                                                     {"b/impl.rb", 7, "resolves but its package is not imported"},
                                                     {"c/__package.rb", 0, "must contain a package definition"},
                                                 });
}

} // namespace sorbet::test::lsp
