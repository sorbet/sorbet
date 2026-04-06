#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "ast/Helpers.h"
#include "ast/ast.h"

namespace sorbet::ast {

TEST_CASE("SwappingParsedTreesWorks") {
    core::FileRef leftFile(1);
    core::FileRef rightFile(2);

    core::LocalVariable leftVar(core::Names::it(), 0);
    core::LocalVariable rightVar(core::Names::it(), 1);

    ParsedFile left(make_expression<Local>(core::LocOffsets::none(), leftVar), leftFile);
    ParsedFile right(make_expression<Local>(core::LocOffsets::none(), rightVar), rightFile);

    left.setCached(true);

    left.swap(right);

    REQUIRE(isa_tree<Local>(left.tree));
    REQUIRE(isa_tree<Local>(right.tree));

    CHECK_EQ(cast_tree<Local>(left.tree)->localVariable, rightVar);
    CHECK_EQ(cast_tree<Local>(right.tree)->localVariable, leftVar);

    CHECK_EQ(left.file, rightFile);
    CHECK_EQ(right.file, leftFile);

    CHECK(!left.cached());
    CHECK(right.cached());
}

} // namespace sorbet::ast
