#include "DefLocSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/Query.h"
#include "core/lsp/QueryResponse.h"

using namespace std;
namespace sorbet::realmain::lsp {

ast::ExpressionPtr DefLocSaver::postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
    auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);

    const core::lsp::Query &lspQuery = ctx.state.lspQuery;
    bool lspQueryMatch =
        lspQuery.matchesLoc(core::Loc(ctx.file, methodDef.declLoc)) || lspQuery.matchesSymbol(methodDef.symbol);

    if (lspQueryMatch) {
        // Query matches against the method definition as a whole.
        auto symbolData = methodDef.symbol.data(ctx);
        auto &argTypes = symbolData->arguments;
        core::TypeAndOrigins tp;

        // Check if it matches against a specific argument. If it does, send that instead;
        // it's more specific.
        const int numArgs = methodDef.args.size();

        ENFORCE(numArgs == argTypes.size());
        for (int i = 0; i < numArgs; i++) {
            auto &arg = methodDef.args[i];
            auto &argType = argTypes[i];
            auto *localExp = ast::MK::arg2Local(arg);
            // localExp should never be null, but guard against the possibility.
            if (localExp && lspQuery.matchesLoc(core::Loc(ctx.file, localExp->loc))) {
                tp.type = argType.type;
                tp.origins.emplace_back(core::Loc(ctx.file, localExp->loc));
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx, core::lsp::IdentResponse(core::Loc(ctx.file, localExp->loc), localExp->localVariable, tp,
                                                  methodDef.symbol));
                return tree;
            }
        }

        tp.type = symbolData->resultType;
        tp.origins.emplace_back(core::Loc(ctx.file, methodDef.declLoc));
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::DefinitionResponse(methodDef.symbol, core::Loc(ctx.file, methodDef.declLoc), methodDef.name,
                                               tp));
    }

    return tree;
}

ast::ExpressionPtr DefLocSaver::postTransformUnresolvedIdent(core::Context ctx, ast::ExpressionPtr tree) {
    auto &id = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
    if (id.kind == ast::UnresolvedIdent::Kind::Instance || id.kind == ast::UnresolvedIdent::Kind::Class) {
        core::ClassOrModuleRef klass;
        // Logic cargo culted from `global2Local` in `walker_build.cc`.
        if (id.kind == ast::UnresolvedIdent::Kind::Instance) {
            ENFORCE(ctx.owner.isMethod());
            klass = ctx.owner.owner(ctx).asClassOrModuleRef();
        } else {
            // Class var.
            klass = ctx.owner.enclosingClass(ctx);
            while (klass.data(ctx)->attachedClass(ctx).exists()) {
                klass = klass.data(ctx)->attachedClass(ctx);
            }
        }

        auto sym = klass.data(ctx)->findMemberTransitive(ctx, id.name);
        const core::lsp::Query &lspQuery = ctx.state.lspQuery;
        if (sym.exists() && sym.isFieldOrStaticField() &&
            (lspQuery.matchesSymbol(sym) || lspQuery.matchesLoc(core::Loc(ctx.file, id.loc)))) {
            auto field = sym.asFieldRef();
            core::TypeAndOrigins tp;
            tp.type = field.data(ctx)->resultType;
            tp.origins.emplace_back(field.data(ctx)->loc());
            core::lsp::QueryResponse::pushQueryResponse(
                ctx, core::lsp::FieldResponse(field, core::Loc(ctx.file, id.loc), id.name, tp));
        }
    }
    return tree;
}

namespace {

// Keep in mind that DefLocSaver runs before class_flatten runs, so code is not already inside
// static-init methods necessarily.
core::MethodRef enclosingMethodFromContext(core::Context ctx) {
    if (ctx.owner.isMethod()) {
        return ctx.owner.asMethodRef();
    } else if (ctx.owner.isClassOrModule()) {
        if (ctx.owner == core::Symbols::root()) {
            return ctx.state.lookupStaticInitForFile(ctx.file);
        } else {
            return ctx.state.lookupStaticInitForClass(ctx.owner.asClassOrModuleRef());
        }
    } else {
        ENFORCE(false, "Unexpected owner in context: {}", ctx.owner.show(ctx));
        return core::MethodRef{};
    }
}

void matchesQuery(core::Context ctx, ast::ConstantLit *lit, const core::lsp::Query &lspQuery,
                  core::SymbolRef symbolBeforeDealias) {
    // Iterate. Ensures that we match "Foo" in "Foo::Bar" references.
    auto symbol = symbolBeforeDealias.dealias(ctx);
    while (lit && symbol.exists() && lit->original) {
        auto &unresolved = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(lit->original);
        if (lspQuery.matchesLoc(core::Loc(ctx.file, lit->loc)) || lspQuery.matchesSymbol(symbol)) {
            // This basically approximates the cfg::Alias case from Environment::processBinding.
            core::TypeAndOrigins tp;
            tp.origins.emplace_back(symbol.loc(ctx));

            if (symbol.isClassOrModule()) {
                tp.type = symbol.asClassOrModuleRef().data(ctx)->lookupSingletonClass(ctx).data(ctx)->externalType();
            } else {
                auto resultType = symbol.resultType(ctx);
                tp.type = resultType == nullptr ? core::Types::untyped(ctx, symbol) : resultType;
            }

            core::lsp::ConstantResponse::Scopes scopes;
            if (symbol == core::Symbols::StubModule()) {
                scopes = *lit->resolutionScopes;
            } else {
                scopes = {symbol.owner(ctx)};
            }

            auto enclosingMethod = enclosingMethodFromContext(ctx);
            auto resp = core::lsp::ConstantResponse(symbol, symbolBeforeDealias, core::Loc(ctx.file, lit->loc), scopes,
                                                    unresolved.cnst, tp, enclosingMethod);
            core::lsp::QueryResponse::pushQueryResponse(ctx, resp);
        }
        lit = ast::cast_tree<ast::ConstantLit>(unresolved.scope);
        if (lit) {
            symbolBeforeDealias = lit->symbol;
            symbol = symbolBeforeDealias.dealias(ctx);
        }
    }
}

} // namespace

ast::ExpressionPtr DefLocSaver::postTransformConstantLit(core::Context ctx, ast::ExpressionPtr tree) {
    auto &lit = ast::cast_tree_nonnull<ast::ConstantLit>(tree);
    const core::lsp::Query &lspQuery = ctx.state.lspQuery;
    matchesQuery(ctx, &lit, lspQuery, lit.symbol);
    return tree;
}

} // namespace sorbet::realmain::lsp
