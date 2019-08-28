#ifndef SORBET_DSL_MODULE_FUNCTION_H
#define SORBET_DSL_MODULE_FUNCTION_H
#include "ast/ast.h"

namespace sorbet::dsl {

class ModuleFunction final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::Send *send, const ast::Expression *prevStat);

    ModuleFunction() = delete;
};

} // namespace sorbet::dsl

#endif
