#ifndef SORBET_LOCAL_VARS_H
#define SORBET_LOCAL_VARS_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::local_vars {

class LocalVars final {
public:
    static std::unique_ptr<ast::Expression> run(core::MutableContext ctx, std::unique_ptr<ast::Expression> expr);

    LocalVars() = delete;
};

} // namespace sorbet::local_vars

#endif
