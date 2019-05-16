#ifndef SORBET_NAME_LOCALS_H
#define SORBET_NAME_LOCALS_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::name_locals {

class NameLocals final {
public:
    static std::unique_ptr<ast::Expression> run(core::MutableContext ctx, std::unique_ptr<ast::Expression> expr);

    NameLocals() = delete;
};

} // namespace sorbet::name_locals

#endif
