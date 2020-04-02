#ifndef SORBET_REWRITER_UTIL_H
#define SORBET_REWRITER_UTIL_H

#include "ast/ast.h"
#include "core/Names.h"
#include <memory>

namespace sorbet::rewriter {
class ASTUtil {
public:
    static std::unique_ptr<ast::Expression> dupType(const ast::Expression *orig);
    static bool hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);
    static bool hasTruthyHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name);

    /** Removes the key, value matching key with symbol `name` from hash and returns it */
    static std::pair<std::unique_ptr<ast::Expression>, std::unique_ptr<ast::Expression>>
    extractHashValue(core::MutableContext ctx, ast::Hash &hash, core::NameRef name);

    static void putBackHashValue(core::MutableContext ctx, ast::Hash &hash, std::unique_ptr<ast::Expression> key,
                                 std::unique_ptr<ast::Expression> value);
    static const ast::Send *castSig(const ast::Expression *expr, core::NameRef returns);

    static std::unique_ptr<ast::Expression> mkGet(core::Loc loc, core::NameRef name,
                                                  std::unique_ptr<ast::Expression> rhs);

    static std::unique_ptr<ast::Expression> mkSet(core::Loc loc, core::NameRef name, core::LocOffsets argLoc,
                                                  std::unique_ptr<ast::Expression> rhs);

    static std::unique_ptr<ast::Expression> mkNilable(core::LocOffsets loc, std::unique_ptr<ast::Expression> type);

    static std::unique_ptr<ast::Expression> mkMutator(core::MutableContext ctx, core::LocOffsets loc,
                                                      core::NameRef className);

    static std::unique_ptr<ast::Expression> thunkBody(core::MutableContext ctx, ast::Expression *node);

    static bool isProbablySymbol(core::MutableContext ctx, ast::Expression *type, core::SymbolRef sym);

    ASTUtil() = delete;
};
} // namespace sorbet::rewriter
#endif
