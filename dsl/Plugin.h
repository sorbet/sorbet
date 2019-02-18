#ifndef SORBET_DSL_PLUGIN_H
#define SORBET_DSL_PLUGIN_H
#include "ast/ast.h"

namespace sorbet::dsl {

class Plugin final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, std::unique_ptr<ast::ClassDef> &classDef, ast::Send *send);

    Plugin() = delete;
};

} // namespace sorbet::dsl

#endif
