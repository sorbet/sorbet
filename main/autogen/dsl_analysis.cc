#include "main/autogen/dsl_analysis.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/formatting.h"
#include "main/autogen/crc_builder.h"

using namespace std;
namespace sorbet::autogen {

const std::vector<u4> KNOWN_PROP_METHODS = {
    core::Names::prop().rawId(),         core::Names::tokenProp().rawId(),
    core::Names::tokenProp().rawId(),    core::Names::timestampedTokenProp().rawId(),
    core::Names::createdProp().rawId(),  core::Names::updatedProp().rawId(),
    core::Names::merchantProp().rawId(), core::Names::merchantTokenProp().rawId(),
    core::Names::const_().rawId(),
};

struct PropInfoInternal {
    core::NameRef name;
    std::optional<ast::ExpressionPtr> typeExp;
};

class DSLAnalysisWalk {
    UnorderedMap<vector<core::NameRef>, DSLInfo> dslInfo;
    vector<vector<core::NameRef>> nestingScopes;
    core::FileRef file;
    bool validScope;

    // Convert a symbol name into a fully qualified name
    vector<core::NameRef> symbolName(core::Context ctx, core::SymbolRef sym) {
        vector<core::NameRef> out;
        while (sym.exists() && sym != core::Symbols::root()) {
            out.emplace_back(sym.data(ctx)->name);
            sym = sym.data(ctx)->owner;
        }
        reverse(out.begin(), out.end());
        return out;
    }

    std::optional<PropInfoInternal> parseProp(core::Context ctx, ast::Send *send) {
        switch (send->fun.rawId()) {
            case core::Names::const_().rawId():
            case core::Names::prop().rawId(): {
                auto *lit = ast::cast_tree<ast::Literal>(send->args.front());
                if (lit && lit->isSymbol(ctx)) {
                    if (send->args.size() > 1) {
                        auto maybeTransformedType = transformTypeForMutator(ctx, send->args[1]);
                        if (maybeTransformedType.has_value()) {
                            return PropInfoInternal{lit->asSymbol(ctx), std::move(*maybeTransformedType)};
                        } else {
                            return PropInfoInternal{lit->asSymbol(ctx), std::move(send->args[1])};
                        }
                    }

                    return PropInfoInternal{lit->asSymbol(ctx), std::nullopt};
                }

                break;
            }
            case core::Names::tokenProp().rawId():
            case core::Names::timestampedTokenProp().rawId():
                return PropInfoInternal{
                    core::Names::token(),
                    ast::MK::Constant(send->loc, core::Symbols::String()),
                };
            case core::Names::createdProp().rawId():
                return PropInfoInternal{core::Names::created(), ast::MK::Constant(send->loc, core::Symbols::Float())};
            case core::Names::updatedProp().rawId():
                return PropInfoInternal{core::Names::updated(), ast::MK::Constant(send->loc, core::Symbols::Float())};
            case core::Names::merchantProp().rawId():
                return PropInfoInternal{
                    core::Names::merchant(),
                    ast::MK::Constant(send->loc, core::Symbols::String()),
                };
            case core::Names::merchantTokenProp().rawId():
                return PropInfoInternal{
                    core::Names::merchant(),
                    ast::MK::UnresolvedConstant(
                        send->loc,
                        ast::MK::UnresolvedConstant(
                            send->loc,
                            ast::MK::UnresolvedConstant(send->loc,
                                                        ast::MK::UnresolvedConstant(send->loc, ast::MK::EmptyTree(),
                                                                                    core::Names::Constants::Opus()),
                                                        core::Names::Constants::Autogen()),
                            core::Names::Constants::Tokens()),
                        core::Names::Constants::AccountModelMerchantToken()),
                };
            default:
                return std::nullopt;
        }

        return std::nullopt;
    }

    std::optional<ast::ExpressionPtr> transformTypeForMutator(core::Context ctx, ast::ExpressionPtr &propType) {
        auto *send = ast::cast_tree<ast::Send>(propType);
        if (send) {
            if (send->fun == core::Names::nilable()) {
                auto innerType = transformTypeForMutator(ctx, send->args[0]);
                if (innerType.has_value()) {
                    auto *innerSend = ast::cast_tree<ast::Send>(*innerType);
                    if (innerSend && innerSend->fun == core::Names::untyped()) {
                        return innerType;
                    }

                    ast::Send::ARGS_store args;
                    args.emplace_back(std::move(*innerType));

                    return ast::MK::Send(propType.loc(), std::move(send->recv), core::Names::nilable(), 1,
                                         std::move(args));
                }
            }

            if (send->fun == core::Names::enum_() || send->fun == core::Names::deprecatedEnum()) {
                return ast::MK::Send(propType.loc(), std::move(send->recv), core::Names::untyped(), 0, {});
            }

            if (send->fun == core::Names::squareBrackets()) {
                // Typed hash, array
                return ast::MK::Send(
                    propType.loc(),
                    ast::MK::UnresolvedConstant(propType.loc(), ast::MK::EmptyTree(), core::Names::Constants::T()),
                    core::Names::untyped(), 0, {});
            }

            return std::nullopt;
        }

        if (ast::cast_tree<ast::Array>(propType) || ast::cast_tree<ast::Hash>(propType)) {
            return ast::MK::Send(
                propType.loc(),
                ast::MK::UnresolvedConstant(propType.loc(), ast::MK::EmptyTree(), core::Names::Constants::T()),
                core::Names::untyped(), 0, {});
        }

        return std::nullopt;
    }

public:
    DSLAnalysisWalk(core::FileRef a_file) {
        validScope = true;
        file = a_file;
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (original.symbol.data(ctx)->owner == core::Symbols::PackageRegistry()) {
            // this is a package, so do not enter a definition for it
            return tree;
        }

        vector<vector<core::NameRef>> ancestors;
        for (auto &ancst : original.ancestors) {
            auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
            if (cnst == nullptr || cnst->original == nullptr) {
                // ignore them if they're not statically-known ancestors (i.e. not constants)
                continue;
            }

            ancestors.emplace_back(symbolName(ctx, cnst->symbol));
        }

        const vector<core::NameRef> className = symbolName(ctx, original.symbol);
        nestingScopes.emplace_back(className);
        dslInfo.emplace(className, DSLInfo{{}, ancestors, file, {}, {}});

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0 || !validScope) {
            // Not in any scope
            return tree;
        }

        nestingScopes.pop_back();

        return tree;
    }

    ast::ExpressionPtr preTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0) {
            // Not in any scope
            return tree;
        }

        auto *original = ast::cast_tree<ast::Send>(tree);
        auto &curScope = nestingScopes.back();

        u4 funId = original->fun.rawId();
        bool isProp = absl::c_any_of(KNOWN_PROP_METHODS, [&](const auto &nrid) -> bool { return nrid == funId; });
        if (isProp) {
            if (!validScope) {
                dslInfo[curScope].problemLocs.emplace_back(LocInfo{file, std::move(original->loc)});
                return tree;
            }

            const auto propInfo = parseProp(ctx, original);
            if (propInfo.has_value()) {
                std::optional<std::string> typeStr;
                if ((*propInfo).typeExp.has_value()) {
                    typeStr = std::move(*((*propInfo).typeExp)).toString(ctx);
                }

                dslInfo[curScope].props.emplace_back(PropInfo{std::move((*propInfo).name), std::move(typeStr)});
            } else {
                dslInfo[curScope].problemLocs.emplace_back(LocInfo{file, std::move(original->loc)});
            }

            return tree;
        }

        if (original->fun == core::Names::modelDsl()) {
            auto *cnst = ast::cast_tree<ast::ConstantLit>(original->args.front());
            if (!validScope || cnst == nullptr || cnst->original == nullptr) {
                return tree;
            }

            dslInfo[curScope].model = symbolName(ctx, cnst->symbol);
        }

        return tree;
    }

    ast::ExpressionPtr preTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0 || !validScope) {
            // Not already in a valid scope
            return tree;
        }

        validScope = false;
        return tree;
    }

    ast::ExpressionPtr postTransformMethodDef(core::Context ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0 || validScope) {
            // Already in a valid scope, or never in a scope
            return tree;
        }

        validScope = true;
        return tree;
    }

    DSLAnalysisFile dslAnalysisFile() {
        DSLAnalysisFile out;
        out.dslInfo = std::move(dslInfo);
        out.file = std::move(file);
        return out;
    }
};

DSLAnalysisFile DSLAnalysis::generate(core::Context ctx, ast::ParsedFile tree, const CRCBuilder &crcBuilder) {
    DSLAnalysisWalk walk(tree.file);
    ast::TreeMap::apply(ctx, walk, move(tree.tree));
    auto daf = walk.dslAnalysisFile();
    auto src = tree.file.data(ctx).source();
    daf.cksum = crcBuilder.crc32(src);
    return daf;
}

} // namespace sorbet::autogen
