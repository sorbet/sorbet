#ifndef SORBET_DSL_HELPERS
#define SORBET_DSL_HELPERS
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/core.h"
#include "dsl/util.h"

namespace sorbet::dsl {

std::unique_ptr<ast::Expression> mkGet(core::Loc loc, core::NameRef name, std::unique_ptr<ast::Expression> rhs);

std::unique_ptr<ast::Expression> mkSet(core::Loc loc, core::NameRef name, core::Loc nameLoc,
                                       std::unique_ptr<ast::Expression> rhs);

std::unique_ptr<ast::Expression> mkNilable(core::Loc loc, std::unique_ptr<ast::Expression> type);

std::unique_ptr<ast::Expression> mkMutator(core::MutableContext ctx, core::Loc loc, core::NameRef className);

std::unique_ptr<ast::Expression> thunkBody(core::MutableContext ctx, ast::Expression *node);

bool isProbablySymbol(core::MutableContext ctx, ast::Expression *type, core::SymbolRef sym);

} // namespace sorbet::dsl

#endif
