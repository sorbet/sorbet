#ifndef SORBET_DSL_MODULE_FUNCTION_H
#define SORBET_DSL_MODULE_FUNCTION_H
#include "ast/ast.h"

namespace sorbet::dsl {

class ModuleFunction final {
public:
    static void patchDSL(core::MutableContext ctx, ast::ClassDef *cdef);
    ModuleFunction() = delete;

private:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, ast::Send *send,
                                                                    const ast::Expression *prevStat);

    static std::vector<std::unique_ptr<ast::Expression>>
    rewriteDefn(core::MutableContext ctx, const ast::Expression *expr, const ast::Expression *prevStat);

    static bool isBareModuleFunction(const ast::Expression *expr);
};

} // namespace sorbet::dsl

#endif
