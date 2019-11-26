#ifndef SORBET_REWRITER_INITIALIZER_H
#define SORBET_REWRITER_INITIALIZER_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Initializer final {
public:
    static void run(core::MutableContext ctx, ast::MethodDef *methodDef, const ast::Expression *prevStat);

    Initializer() = delete;
};

} // namespace sorbet::rewriter

#endif
