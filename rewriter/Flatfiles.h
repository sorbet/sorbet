#ifndef SORBET_REWRITER_FLATFILES_H
#define SORBET_REWRITER_FLATFILES_H
#include "ast/ast.h"

namespace sorbet::rewriter {
class Flatfiles final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    Flatfiles() = delete;
};

} // namespace sorbet::rewriter

#endif
