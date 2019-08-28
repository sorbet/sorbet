#ifndef SORBET_DSL_MODULE_FUNCTION_H
#define SORBET_DSL_MODULE_FUNCTION_H
#include "ast/ast.h"

namespace sorbet::dsl {

class ModuleFunction final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::Send *send,
                                                                    const ast::Expression *prevStat);

    static std::vector<std::unique_ptr<ast::Expression>>
    rewriteDefn(core::MutableContext ctx, const ast::Expression *expr, const ast::Expression *prevStat, bool usedSig);

    static bool isModuleFunction(const ast::Expression *expr);

    ModuleFunction() = delete;
};

} // namespace sorbet::dsl

#endif
