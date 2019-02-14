#ifndef SORBET_DSL_UTIL_H
#define SORBET_DSL_UTIL_H

#include "ast/ast.h"
#include "core/Names.h"
#include <memory>

namespace sorbet::dsl {
class ASTUtil {
public:
    static std::unique_ptr<ast::Expression> dupType(const ast::Expression *orig);
    static bool hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);

    /** Removes the key, value matching key with symbol `name` from hash and returns it */
    static std::pair<std::unique_ptr<ast::Expression>, std::unique_ptr<ast::Expression>>
    extractHashValue(core::MutableContext ctx, ast::Hash &hash, core::NameRef name);

    static void putBackHashValue(core::MutableContext ctx, ast::Hash &hash, std::unique_ptr<ast::Expression> key,
                                 std::unique_ptr<ast::Expression> value);

    ASTUtil() = delete;
};
} // namespace sorbet::dsl
#endif
