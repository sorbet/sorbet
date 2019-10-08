#ifndef SORBET_COMPILER_DEFINITON_REWRITER_H
#define SORBET_COMPILER_DEFINITON_REWRITER_H

namespace sorbet::core {
class MutableContext;
}
namespace sorbet::ast {
class ClassDef;
}

namespace sorbet::compiler {
class DefinitionRewriter {
public:
    static void run(core::MutableContext &ctx, ast::ClassDef *klass);
};
} // namespace sorbet::compiler
#endif
