#include "main/pipeline/definition_checker/DefinitionLinesDenylistEnforcer.h"
#include "ast/treemap/treemap.h"

namespace sorbet::pipeline::definition_checker {

ast::ParsedFile checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, ast::ParsedFile what,
                                                        int prohibitedLinesStart, int prohibitedLinesEnd) {
    DefinitionLinesDenylistEnforcer enforcer(what.file, prohibitedLinesStart, prohibitedLinesEnd);
    ast::TreeWalk::apply(core::Context(gs, core::Symbols::root(), what.file), enforcer, what.tree);
    return what;
}
bool DefinitionLinesDenylistEnforcer::isAllowListed(core::Context ctx, core::SymbolRef sym) {
    return sym.name(ctx) == core::Names::staticInit() || sym.name(ctx) == core::Names::Constants::Root() ||
           sym.name(ctx) == core::Names::unresolvedAncestors();
}

void DefinitionLinesDenylistEnforcer::checkLoc(core::Context ctx, core::Loc loc) {
    auto detailStart = core::Loc::offset2Pos(file.data(ctx), loc.beginPos());
    auto detailEnd = core::Loc::offset2Pos(file.data(ctx), loc.endPos());
    ENFORCE(!(detailStart.line >= prohibitedLinesStart && detailEnd.line <= prohibitedLinesEnd));
}

void DefinitionLinesDenylistEnforcer::checkSym(core::Context ctx, core::SymbolRef sym) {
    if (isAllowListed(ctx, sym)) {
        return;
    }
    checkLoc(ctx, sym.loc(ctx));
}

void DefinitionLinesDenylistEnforcer::preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
    checkSym(ctx, ast::cast_tree_nonnull<ast::ClassDef>(tree).symbol);
}
void DefinitionLinesDenylistEnforcer::preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
    checkSym(ctx, ast::cast_tree_nonnull<ast::MethodDef>(tree).symbol);
}
} // namespace sorbet::pipeline::definition_checker
