#include "DefLocSaver.h"
#include "ast/Helpers.h"
#include "core/lsp/Query.h"
#include "core/lsp/QueryResponse.h"

using namespace std;
namespace sorbet::realmain::lsp {

unique_ptr<ast::MethodDef> DefLocSaver::postTransformMethodDef(core::Context ctx,
                                                               unique_ptr<ast::MethodDef> methodDef) {
    const core::lsp::Query &lspQuery = ctx.state.lspQuery;
    bool lspQueryMatch = lspQuery.matchesLoc(methodDef->declLoc) || lspQuery.matchesSymbol(methodDef->symbol);

    if (lspQueryMatch) {
        // Query matches against the method definition as a whole.
        auto &symbolData = methodDef->symbol.data(ctx);
        auto &argTypes = symbolData->arguments();
        core::TypeAndOrigins tp;

        // Check if it matches against a specific argument. If it does, send that instead;
        // it's more specific.
        const int numArgs = methodDef->args.size();

        ENFORCE(numArgs == argTypes.size());
        for (int i = 0; i < numArgs; i++) {
            auto &arg = methodDef->args[i];
            auto &argType = argTypes[i];
            auto *localExp = ast::MK::arg2Local(arg.get());
            // localExp should never be null, but guard against the possibility.
            if (localExp && lspQuery.matchesLoc(core::Loc(ctx.file, localExp->loc))) {
                tp.type = argType.type;
                tp.origins.emplace_back(core::Loc(ctx.file, localExp->loc));
                core::lsp::QueryResponse::pushQueryResponse(
                    ctx, core::lsp::IdentResponse(core::Loc(ctx.file, localExp->loc), localExp->localVariable, tp,
                                                  methodDef->symbol));
                return methodDef;
            }
        }

        tp.type = symbolData->resultType;
        tp.origins.emplace_back(methodDef->declLoc);
        core::lsp::QueryResponse::pushQueryResponse(
            ctx, core::lsp::DefinitionResponse(methodDef->symbol, methodDef->declLoc, methodDef->name, tp));
    }

    return methodDef;
}

unique_ptr<ast::UnresolvedIdent> DefLocSaver::postTransformUnresolvedIdent(core::Context ctx,
                                                                           unique_ptr<ast::UnresolvedIdent> id) {
    if (id->kind == ast::UnresolvedIdent::Kind::Instance || id->kind == ast::UnresolvedIdent::Kind::Class) {
        core::SymbolRef klass;
        // Logic cargo culted from `global2Local` in `walker_build.cc`.
        if (id->kind == ast::UnresolvedIdent::Kind::Instance) {
            ENFORCE(ctx.owner.data(ctx)->isMethod());
            klass = ctx.owner.data(ctx)->owner;
        } else {
            // Class var.
            klass = ctx.owner.data(ctx)->enclosingClass(ctx);
            while (klass.data(ctx)->attachedClass(ctx, klass).exists()) {
                klass = klass.data(ctx)->attachedClass(ctx, klass);
            }
        }

        auto sym = klass.data(ctx)->findMemberTransitive(ctx, id->name);
        const core::lsp::Query &lspQuery = ctx.state.lspQuery;
        if (sym.exists() && (lspQuery.matchesSymbol(sym) || lspQuery.matchesLoc(core::Loc(ctx.file, id->loc)))) {
            core::TypeAndOrigins tp;
            tp.type = sym.data(ctx)->resultType;
            tp.origins.emplace_back(sym.data(ctx)->loc());
            core::lsp::QueryResponse::pushQueryResponse(
                ctx, core::lsp::FieldResponse(sym, core::Loc(ctx.file, id->loc), id->name, tp));
        }
    }
    return id;
}

void matchesQuery(core::Context ctx, ast::ConstantLit *lit, const core::lsp::Query &lspQuery, core::SymbolRef symbol) {
    // Iterate. Ensures that we match "Foo" in "Foo::Bar" references.
    while (lit && symbol.exists() && lit->original) {
        if (lspQuery.matchesLoc(core::Loc(ctx.file, lit->loc)) || lspQuery.matchesSymbol(symbol)) {
            // This basically approximates the cfg::Alias case from Environment::processBinding.
            core::TypeAndOrigins tp;
            tp.origins.emplace_back(symbol.data(ctx)->loc());

            if (symbol.data(ctx)->isClassOrModule()) {
                auto singletonClass = symbol.data(ctx)->lookupSingletonClass(ctx, symbol);
                tp.type = singletonClass.data(ctx)->externalType(ctx, singletonClass);
            } else {
                auto resultType = symbol.data(ctx)->resultType;
                tp.type = resultType == nullptr ? core::Types::untyped(ctx, symbol) : resultType;
            }

            core::lsp::ConstantResponse::Scopes scopes;
            if (symbol == core::Symbols::StubModule()) {
                scopes = lit->resolutionScopes;
            } else {
                scopes = {symbol.data(ctx)->owner};
            }

            auto resp =
                core::lsp::ConstantResponse(symbol, core::Loc(ctx.file, lit->loc), scopes, lit->original->cnst, tp);
            core::lsp::QueryResponse::pushQueryResponse(ctx, resp);
        }
        lit = ast::cast_tree<ast::ConstantLit>(lit->original->scope.get());
        if (lit) {
            symbol = lit->symbol.data(ctx)->dealias(ctx);
        }
    }
}

unique_ptr<ast::ConstantLit> DefLocSaver::postTransformConstantLit(core::Context ctx,
                                                                   unique_ptr<ast::ConstantLit> lit) {
    const core::lsp::Query &lspQuery = ctx.state.lspQuery;
    auto symbol = lit->symbol.data(ctx)->dealias(ctx);
    matchesQuery(ctx, lit.get(), lspQuery, symbol);
    return lit;
}

} // namespace sorbet::realmain::lsp
