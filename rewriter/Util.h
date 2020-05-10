#ifndef SORBET_REWRITER_UTIL_H
#define SORBET_REWRITER_UTIL_H

#include "ast/ast.h"
#include "core/Names.h"
#include <memory>

namespace sorbet::rewriter {
class ASTUtil {
public:
    static ast::TreePtr dupType(const ast::TreePtr &orig);
    static bool hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);
    static bool hasTruthyHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);

    /** Removes the key, value matching key with symbol `name` from hash and returns it */
    static std::pair<ast::TreePtr, ast::TreePtr> extractHashValue(core::MutableContext ctx, ast::Hash &hash,
                                                                  core::NameRef name);

    static ast::Send *castSig(ast::TreePtr &expr, core::NameRef returns);
    static ast::Send *castSig(ast::Send *expr, core::NameRef returns);

    static ast::TreePtr mkGet(core::Context ctx, core::LocOffsets loc, core::NameRef name, ast::TreePtr rhs);

    static ast::TreePtr mkSet(core::Context ctx, core::LocOffsets loc, core::NameRef name, core::LocOffsets argLoc,
                              ast::TreePtr rhs);

    static ast::TreePtr mkNilable(core::LocOffsets loc, ast::TreePtr type);

    static ast::TreePtr mkMutator(core::MutableContext ctx, core::LocOffsets loc, core::NameRef className);

    static ast::TreePtr thunkBody(core::MutableContext ctx, ast::TreePtr &node);

    static bool isProbablySymbol(core::MutableContext ctx, ast::TreePtr &type, core::SymbolRef sym);

    ASTUtil() = delete;
};
} // namespace sorbet::rewriter
#endif
