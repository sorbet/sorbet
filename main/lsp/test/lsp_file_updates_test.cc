#include "doctest.h"
// has to go first as it violates our requirements

#include "ast/ast.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPFileUpdates.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp::test {

using namespace std;
using namespace sorbet::realmain::lsp;

namespace {
u4 nextHash = 0;
unique_ptr<core::FileHash> getFileHash() {
    auto hash = make_unique<core::FileHash>();
    hash->definitions.hierarchyHash = nextHash++;
    return hash;
}

ast::ParsedFile getParsedFile(core::FileRef fref) {
    auto lit = ast::make_tree<ast::Literal>(core::LocOffsets{0, 1}, core::Types::Integer());
    return ast::ParsedFile{move(lit), fref};
}

shared_ptr<core::File> makeFile(std::string path, std::string contents) {
    auto file = make_shared<core::File>(move(path), move(contents), core::File::Type::Normal);
    file->setFileHash(getFileHash());
    return file;
}

void addFile(LSPFileUpdates &updates, core::FileRef fref, std::string path, std::string contents) {
    updates.updatedFiles.push_back(makeFile(move(path), move(contents)));
    updates.updatedFileIndexes.push_back(getParsedFile(fref));
}

} // namespace

TEST_CASE("MergeOlderVersions") {
    LSPFileUpdates oldUpdates;
    oldUpdates.editCount = 6;
    oldUpdates.epoch = 10;

    LSPFileUpdates newUpdates;
    newUpdates.editCount = 4;
    newUpdates.epoch = 14;

    newUpdates.mergeOlder(oldUpdates);

    CHECK_EQ(10, newUpdates.editCount);
    CHECK_EQ(14, newUpdates.epoch);
}

TEST_CASE("MergeOlderCancellationExpected") {
    LSPFileUpdates oldUpdates;
    oldUpdates.cancellationExpected = true;

    LSPFileUpdates newUpdates;
    newUpdates.cancellationExpected = false;

    newUpdates.mergeOlder(oldUpdates);
    CHECK(newUpdates.cancellationExpected);

    oldUpdates.cancellationExpected = false;
    newUpdates.mergeOlder(oldUpdates);
    CHECK(newUpdates.cancellationExpected);

    newUpdates.cancellationExpected = false;
    newUpdates.mergeOlder(oldUpdates);
    CHECK_FALSE(newUpdates.cancellationExpected);
}

TEST_CASE("MergeOlderHasNewFiles") {
    LSPFileUpdates oldUpdates;
    oldUpdates.hasNewFiles = true;

    LSPFileUpdates newUpdates;
    newUpdates.hasNewFiles = false;

    newUpdates.mergeOlder(oldUpdates);
    CHECK(newUpdates.hasNewFiles);

    oldUpdates.hasNewFiles = false;
    newUpdates.mergeOlder(oldUpdates);
    CHECK(newUpdates.hasNewFiles);

    newUpdates.hasNewFiles = false;
    newUpdates.mergeOlder(oldUpdates);
    CHECK_FALSE(newUpdates.hasNewFiles);
}

TEST_CASE("MergeUpdatedFiles") {
    LSPFileUpdates oldUpdates;
    oldUpdates.editCount = 6;
    oldUpdates.epoch = 10;
    addFile(oldUpdates, core::FileRef(1), "foo.rb", "foo");
    addFile(oldUpdates, core::FileRef(2), "bar.rb", "oldcontents");

    LSPFileUpdates newUpdates;
    newUpdates.editCount = 4;
    newUpdates.epoch = 14;
    addFile(newUpdates, core::FileRef(2), "bar.rb", "newcontents");
    addFile(newUpdates, core::FileRef(3), "baz.rb", " ");

    newUpdates.mergeOlder(oldUpdates);
    REQUIRE_EQ(3, newUpdates.updatedFiles.size());
    REQUIRE_EQ(3, newUpdates.updatedFileIndexes.size());

    UnorderedMap<string, int> fileIndexes;
    int i = -1;
    for (auto &f : newUpdates.updatedFiles) {
        i++;
        auto path = string(f->path());
        CHECK_FALSE(fileIndexes.contains(path));
        fileIndexes[move(path)] = i;
    }
    {
        REQUIRE(fileIndexes.contains("bar.rb"));
        int i = fileIndexes["bar.rb"];
        CHECK_EQ("newcontents", newUpdates.updatedFiles[i]->source());
        CHECK_EQ(2, newUpdates.updatedFileIndexes[i].file.id());
        CHECK_NE(oldUpdates.updatedFiles[1]->getFileHash()->definitions.hierarchyHash,
                 newUpdates.updatedFiles[i]->getFileHash()->definitions.hierarchyHash);
    }

    {
        REQUIRE(fileIndexes.contains("foo.rb"));
        int i = fileIndexes["foo.rb"];
        CHECK_EQ("foo", newUpdates.updatedFiles[i]->source());
        CHECK_EQ(1, newUpdates.updatedFileIndexes[i].file.id());
        CHECK_EQ(oldUpdates.updatedFiles[0]->getFileHash()->definitions.hierarchyHash,
                 newUpdates.updatedFiles[i]->getFileHash()->definitions.hierarchyHash);
    }

    {
        REQUIRE(fileIndexes.contains("baz.rb"));
        CHECK_EQ(" ", newUpdates.updatedFiles[fileIndexes["baz.rb"]]->source());
    }
}

TEST_CASE("Copy") {
    LSPFileUpdates updates;
    updates.editCount = 10;
    updates.epoch = 10;
    updates.cancellationExpected = true;
    updates.canTakeFastPath = true;
    updates.hasNewFiles = true;
    updates.updatedGS = unique_ptr<core::GlobalState>(nullptr);
    addFile(updates, core::FileRef(1), "foo.rb", "foo");
    addFile(updates, core::FileRef(2), "bar.rb", "bar");

    LSPFileUpdates copy = updates.copy();
    CHECK_EQ(10, copy.epoch);
    CHECK_EQ(10, copy.editCount);
    CHECK(copy.cancellationExpected);
    CHECK(copy.canTakeFastPath);
    CHECK(copy.hasNewFiles);
    CHECK_FALSE(copy.updatedGS.has_value());

    REQUIRE_EQ(2, copy.updatedFiles.size());
    REQUIRE_EQ(2, copy.updatedFileIndexes.size());

    CHECK_EQ("foo.rb", copy.updatedFiles[0]->path());
    CHECK_EQ("foo", copy.updatedFiles[0]->source());
    CHECK_EQ("bar.rb", copy.updatedFiles[1]->path());
    CHECK_EQ("bar", copy.updatedFiles[1]->source());

    CHECK_EQ(1, copy.updatedFileIndexes[0].file.id());
    CHECK_EQ(2, copy.updatedFileIndexes[1].file.id());

    CHECK_EQ(updates.updatedFiles[0]->getFileHash()->definitions.hierarchyHash,
             copy.updatedFiles[0]->getFileHash()->definitions.hierarchyHash);
    CHECK_EQ(updates.updatedFiles[1]->getFileHash()->definitions.hierarchyHash,
             copy.updatedFiles[1]->getFileHash()->definitions.hierarchyHash);
}

TEST_CASE("MergeOlderPreemptionExpected") {
    LSPFileUpdates oldUpdates;
    oldUpdates.preemptionsExpected = 2;

    LSPFileUpdates newUpdates;
    newUpdates.preemptionsExpected = 5;

    newUpdates.mergeOlder(oldUpdates);
    CHECK_EQ(7, newUpdates.preemptionsExpected);
}

} // namespace sorbet::realmain::lsp::test
