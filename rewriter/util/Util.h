#ifndef SORBET_REWRITER_UTIL_H
#define SORBET_REWRITER_UTIL_H

#include "ast/ast.h"
#include "core/Names.h"
#include <memory>

namespace sorbet::rewriter {
class ASTUtil {
public:
    static ast::ExpressionPtr dupType(const ast::ExpressionPtr &orig);
    static bool hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);
    static bool hasTruthyHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);

    /** Removes the key, value matching key with symbol `name` from hash and returns it */
    static std::pair<ast::ExpressionPtr, ast::ExpressionPtr> extractHashValue(core::MutableContext ctx, ast::Hash &hash,
                                                                              core::NameRef name);

    static ast::Send *castSig(ast::ExpressionPtr &expr);
    static const ast::Send *castSig(const ast::ExpressionPtr &expr);
    static ast::Send *castSig(ast::Send *expr);
    static const ast::Send *castSig(const ast::Send *expr);

    static ast::ExpressionPtr mkKwArgsHash(const ast::Send *send);

    static ast::ExpressionPtr mkGet(core::Context ctx, core::LocOffsets loc, core::NameRef name, ast::ExpressionPtr rhs,
                                    ast::MethodDef::Flags flags = ast::MethodDef::Flags());

    static ast::ExpressionPtr mkSet(core::Context ctx, core::LocOffsets loc, core::NameRef name,
                                    core::LocOffsets argLoc, ast::ExpressionPtr rhs,
                                    ast::MethodDef::Flags flags = ast::MethodDef::Flags());

    static ast::ExpressionPtr mkNilable(core::LocOffsets loc, ast::ExpressionPtr type);

    static ast::ExpressionPtr thunkBody(core::MutableContext ctx, ast::ExpressionPtr &node);

    /// Fetch the name and loc of a symbol or string literal, that's suitable for an `attr`, `attr_reader`,
    /// `attr_writer`, or `attr_accessor`
    static std::pair<core::NameRef, core::LocOffsets> getAttrName(core::MutableContext ctx, core::NameRef attrFun,
                                                                  const ast::ExpressionPtr &name);

    // Test if `expr` is a chain of `UnresolvedConstantLit` trees with names equal to `constantName`.
    //
    // 1. Opus::Command as `expr` would match {Constants::Opus(), Constants::Command()}
    // 2. ::Opus::Command as `expr` would match {Constants::Opus(), Constants::Command()}
    // 3. Opus::Command::DoThing as `expr` would not match {Constants::Opus(), Constants::Command()}
    // 4. Prelude::Opus::Command as `expr` would not match {Constants::Opus(), Constants::Command()}
    static bool isRootScopedSyntacticConstant(const ast::ExpressionPtr &expr,
                                              absl::Span<const core::NameRef> constantName);
    struct DuplicateArg {
        core::NameRef name;
        core::LocOffsets firstLoc;
        core::LocOffsets secondLoc;
    };
    static std::optional<DuplicateArg> findDuplicateArg(core::MutableContext ctx, const ast::Send *send);

    ASTUtil() = delete;
};
} // namespace sorbet::rewriter
#endif
