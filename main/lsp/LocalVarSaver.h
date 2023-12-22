#ifndef RUBY_TYPER_LSP_IDENTSAVER_H
#define RUBY_TYPER_LSP_IDENTSAVER_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/core.h"
#include "core/sig_finder/sig_finder.h"

namespace sorbet::realmain::lsp {

class LocalVarSaver {
    std::vector<core::Loc> enclosingMethodDefLoc;
    std::optional<resolver::ParsedSig> signature;

public:
    LocalVarSaver(core::Loc rootLoc, std::optional<resolver::ParsedSig> &&signature)
        : enclosingMethodDefLoc({rootLoc}), signature(move(signature)) {}

    void postTransformBlock(core::Context ctx, const ast::Block &local);
    void postTransformLocal(core::Context ctx, const ast::Local &local);
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
    void postTransformMethodDef(core::Context ctx, const ast::MethodDef &methodDef);
};
}; // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_IDENTSAVER_H
