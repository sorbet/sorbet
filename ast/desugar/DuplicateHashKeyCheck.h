#ifndef SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H
#define SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H

#include "ast/Trees.h"
#include "common/common.h"

namespace sorbet::ast::desugar {

class DuplicateHashKeyCheck {
    const core::MutableContext ctx;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeySymbols;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeyStrings;

public:
    DuplicateHashKeyCheck(core::MutableContext ctx) : ctx{ctx}, hashKeySymbols(), hashKeyStrings() {}

    void check(const ExpressionPtr &key);

    // This is only used with Send::ARGS_store and Array::ELEMS_store
    static void checkSendArgs(const core::MutableContext ctx, int numPosArgs, absl::Span<const ExpressionPtr> args);
};

} // namespace sorbet::ast::desugar

#endif // SORBET_AST_DUPLICATE_HASH_KEY_CHECK_H
