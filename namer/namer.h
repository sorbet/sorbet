#ifndef SORBET_NAMER_NAMER_H
#define SORBET_NAMER_NAMER_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::namer {

class NamerCtx final {
    friend class NameInserter;

private:
    UnorderedMap<core::SymbolRef, core::Loc> classBehaviorLocs;
};

class Namer final {
public:
    static ast::ParsedFile run(core::MutableContext ctx, ast::ParsedFile tree);

    static ast::ParsedFile run(core::MutableContext ctx, std::shared_ptr<NamerCtx> namerCtx, ast::ParsedFile tree);

    Namer() = delete;
};

} // namespace sorbet::namer

#endif
