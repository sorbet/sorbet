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
    static ast::Send *castSig(ast::Send *expr);

    static ast::ExpressionPtr mkKwArgsHash(const ast::Send *send);

    static ast::ExpressionPtr mkGet(core::Context ctx, core::LocOffsets loc, core::NameRef name, ast::ExpressionPtr rhs,
                                    ast::MethodDef::Flags flags = ast::MethodDef::Flags());

    static ast::ExpressionPtr mkSet(core::Context ctx, core::LocOffsets loc, core::NameRef name,
                                    core::LocOffsets argLoc, ast::ExpressionPtr rhs,
                                    ast::MethodDef::Flags flags = ast::MethodDef::Flags());

    static ast::ExpressionPtr mkNilable(core::LocOffsets loc, ast::ExpressionPtr type);

    static ast::ExpressionPtr thunkBody(core::MutableContext ctx, ast::ExpressionPtr &node);

    ASTUtil() = delete;
};
} // namespace sorbet::rewriter
#endif
