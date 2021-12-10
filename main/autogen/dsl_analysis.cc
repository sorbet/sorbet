#include "main/autogen/dsl_analysis.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/formatting.h"
#include "main/autogen/crc_builder.h"

using namespace std;
namespace sorbet::autogen {

const std::vector<u4> KNOWN_PROP_METHODS = {
    core::Names::prop().rawId(),
    core::Names::tokenProp().rawId(),
    core::Names::tokenProp().rawId(),
    core::Names::timestampedTokenProp().rawId(),
    core::Names::createdProp().rawId(),
    core::Names::updatedProp().rawId(),
    core::Names::merchantProp().rawId(),
    core::Names::merchantTokenProp().rawId(),
    core::Names::const_().rawId(),
    core::Names::encryptedProp().rawId(),
    core::Names::bearerTokenProp().rawId(),
    core::Names::feeRuleProp().rawId(),
    core::Names::modelProp().rawId(),
};

const std::vector<core::NameRef> CHALK_ODM_IMMUTABLE_MODEL = {
    core::Names::Constants::Chalk(), core::Names::Constants::ODM(), core::Names::Constants::ImmutableModel()};

const std::vector<core::NameRef> OPUS_DB_SHARD_BY_MERCHANT = {
    core::Names::Constants::Opus(), core::Names::Constants::DB(), core::Names::Constants::Sharding(),
    core::Names::Constants::ShardByMerchant()};

const std::vector<core::NameRef> OPUS_DB_SHARD_BY_MERCHANT_BASE = {
    core::Names::Constants::Opus(), core::Names::Constants::DB(), core::Names::Constants::Sharding(),
    core::Names::Constants::ShardByMerchantBase()};

const std::vector<core::NameRef> OPUS_STORAGE_WAL_WALABLE = {
    core::Names::Constants::Opus(), core::Names::Constants::Storage(), core::Names::Constants::WAL(),
    core::Names::Constants::WALable()};

const std::vector<core::NameRef> OPUS_STORAGE_WAL_WALABLE_CLASSMETHODS = {
    core::Names::Constants::Opus(), core::Names::Constants::Storage(), core::Names::Constants::WAL(),
    core::Names::Constants::WALable(), core::Names::Constants::ClassMethods()};

const std::string ENCRYPTED_ = "encrypted_";
const std::string UNTYPED_STR = "T.untyped";

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
    vector<core::NameRef> symbolName(core::MutableContext ctx, core::SymbolRef sym) {
        vector<core::NameRef> out;
        while (sym.exists() && sym != core::Symbols::root()) {
            out.emplace_back(sym.data(ctx)->name);
            sym = sym.data(ctx)->owner;
        }
        reverse(out.begin(), out.end());
        return out;
    }

    // Convert a constant literal into a fully qualified name
    vector<core::NameRef> constantName(core::MutableContext ctx, ast::ConstantLit &cnstRef) {
        vector<core::NameRef> out;
        auto *cnst = &cnstRef;
        while (cnst != nullptr && cnst->original != nullptr) {
            auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original);
            out.emplace_back(original.cnst);
            cnst = ast::cast_tree<ast::ConstantLit>(original.scope);
        }
        reverse(out.begin(), out.end());
        return out;
    }

    const std::vector<PropInfoInternal> parseProp(core::MutableContext ctx, ast::Send *send) {
        std::vector<PropInfoInternal> result;

        switch (send->fun.rawId()) {
            case core::Names::const_().rawId():
            case core::Names::prop().rawId(): {
                auto *lit = ast::cast_tree<ast::Literal>(send->args.front());
                if (lit && lit->isSymbol(ctx)) {
                    if (send->args.size() > 1) {
                        auto maybeTransformedType = transformTypeForMutator(ctx, send->args[1]);
                        if (maybeTransformedType.has_value()) {
                            result.emplace_back(PropInfoInternal{lit->asSymbol(ctx), std::move(*maybeTransformedType)});
                        } else {
                            result.emplace_back(PropInfoInternal{lit->asSymbol(ctx), std::move(send->args[1])});
                        }
                    } else {
                        result.emplace_back(PropInfoInternal{lit->asSymbol(ctx), std::nullopt});
                    }
                }

                break;
            }
            case core::Names::bearerTokenProp().rawId(): {
                core::NameRef propSuffix = core::Names::bearerToken();
                std::optional<core::NameRef> propPrefix;
                bool optionalProp = false;
                bool timeForConsumedAt = false;

                if (send->numPosArgs > 0) {
                    auto *lit = ast::cast_tree<ast::Literal>(send->args.front());
                    if (lit && lit->isSymbol(ctx)) {
                        propSuffix = lit->asSymbol(ctx);
                    }
                }

                auto [kwStart, kwEnd] = send->kwArgsRange();
                for (auto i = kwStart; i < kwEnd; i += 2) {
                    auto *labelLit = ast::cast_tree<ast::Literal>(send->args[i]);
                    if (labelLit && labelLit->isSymbol(ctx)) {
                        if (labelLit->asSymbol(ctx) == core::Names::propPrefixOpt()) {
                            auto propLit = ast::cast_tree_nonnull<ast::Literal>(send->args[i + 1]);
                            if (propLit.isString(ctx)) {
                                propPrefix = propLit.asString(ctx);
                            }
                        } else if (labelLit->asSymbol(ctx) == core::Names::optional()) {
                            auto propLit = ast::cast_tree_nonnull<ast::Literal>(send->args[i + 1]);
                            if (propLit.isTrue(ctx)) {
                                optionalProp = true;
                            }
                        } else if (labelLit->asSymbol(ctx) == core::Names::timeForConsumedAt()) {
                            auto propLit = ast::cast_tree_nonnull<ast::Literal>(send->args[i + 1]);
                            if (propLit.isTrue(ctx)) {
                                timeForConsumedAt = true;
                            }
                        }
                    }
                }

                core::NameRef propName;
                core::NameRef versionPropName;
                core::NameRef consumedAtPropName;
                if (propPrefix.has_value()) {
                    propName =
                        ctx.state.enterNameConstant(absl::StrCat((*propPrefix).show(ctx), "_", propSuffix.show(ctx)));
                    versionPropName = ctx.state.enterNameConstant(
                        absl::StrCat((*propPrefix).show(ctx), "_", core::Names::bearerTokenVersion().show(ctx)));
                    consumedAtPropName = ctx.state.enterNameConstant(
                        absl::StrCat((*propPrefix).show(ctx), "_", core::Names::bearerTokenConsumedAt().show(ctx)));
                } else {
                    propName = propSuffix;
                    versionPropName = core::Names::bearerTokenVersion();
                    consumedAtPropName = core::Names::bearerTokenConsumedAt();
                }

                ast::ExpressionPtr typeExp;
                ast::ExpressionPtr versionPropTypeExp;
                if (optionalProp) {
                    ast::Send::ARGS_store args;
                    args.emplace_back(ast::MK::Constant(send->loc, core::Symbols::String()));
                    typeExp = ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                            core::Names::nilable(), 1, std::move(args));

                    ast::Send::ARGS_store argsVer;
                    argsVer.emplace_back(ast::MK::Constant(send->loc, core::Symbols::Integer()));
                    versionPropTypeExp = ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                                       core::Names::nilable(), 1, std::move(argsVer));
                } else {
                    typeExp = ast::MK::Constant(send->loc, core::Symbols::String());
                    versionPropTypeExp = ast::MK::Constant(send->loc, core::Symbols::Integer());
                }

                ast::ExpressionPtr consumedAtPropTypeExp;
                if (timeForConsumedAt) {
                    consumedAtPropTypeExp = ast::MK::Constant(send->loc, core::Symbols::Time());
                } else {
                    consumedAtPropTypeExp = ast::MK::Constant(send->loc, core::Symbols::Float());
                }

                result.emplace_back(PropInfoInternal{std::move(propName), std::move(typeExp)});
                result.emplace_back(PropInfoInternal{std::move(versionPropName), std::move(versionPropTypeExp)});
                result.emplace_back(PropInfoInternal{std::move(consumedAtPropName), std::move(consumedAtPropTypeExp)});

                break;
            }
            case core::Names::modelProp().rawId(): {
                if (send->numPosArgs > 0) {
                    bool optionalProp = false;
                    auto *lit = ast::cast_tree<ast::Literal>(send->args.front());
                    if (lit && lit->isSymbol(ctx)) {
                        core::NameRef propName = lit->asSymbol(ctx);

                        auto [kwStart, kwEnd] = send->kwArgsRange();
                        for (auto i = kwStart; i < kwEnd; i += 2) {
                            auto *labelLit = ast::cast_tree<ast::Literal>(send->args[i]);
                            if (labelLit && labelLit->isSymbol(ctx) &&
                                labelLit->asSymbol(ctx) == core::Names::optional()) {
                                auto propLit = ast::cast_tree_nonnull<ast::Literal>(send->args[i + 1]);
                                if (propLit.isTrue(ctx)) {
                                    optionalProp = true;
                                }
                            }
                        }

                        core::NameRef idPropName = ctx.state.enterNameConstant(absl::StrCat(propName.show(ctx), "_id"));
                        core::NameRef collectionPropName =
                            ctx.state.enterNameConstant(absl::StrCat(propName.show(ctx), "_collection"));

                        ast::ExpressionPtr typeExp;
                        ast::ExpressionPtr collectionTypeExp;
                        if (optionalProp) {
                            ast::Send::ARGS_store args;
                            args.emplace_back(ast::MK::Constant(send->loc, core::Symbols::String()));
                            typeExp = ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                                    core::Names::nilable(), 1, std::move(args));

                            ast::Send::ARGS_store argsC;
                            argsC.emplace_back(ast::MK::Constant(send->loc, core::Symbols::String()));
                            collectionTypeExp =
                                ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                              core::Names::nilable(), 1, std::move(argsC));
                        } else {
                            typeExp = ast::MK::Constant(send->loc, core::Symbols::String());
                            collectionTypeExp = ast::MK::Constant(send->loc, core::Symbols::String());
                        }

                        result.emplace_back(PropInfoInternal{std::move(idPropName), std::move(typeExp)});
                        result.emplace_back(
                            PropInfoInternal{std::move(collectionPropName), std::move(collectionTypeExp)});
                    }
                }

                break;
            }
            case core::Names::encryptedProp().rawId(): {
                auto *lit = ast::cast_tree<ast::Literal>(send->args.front());
                if (lit && lit->isSymbol(ctx)) {
                    ast::Send::ARGS_store args;
                    args.emplace_back(ast::MK::Constant(send->loc, core::Symbols::String()));
                    auto typeExp = ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                                 core::Names::nilable(), 1, std::move(args));
                    core::NameRef name = lit->asSymbol(ctx);

                    core::NameRef encryptedName = ctx.state.enterNameConstant(absl::StrCat(ENCRYPTED_, name.show(ctx)));
                    auto typeExpEnc = ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                                    core::Names::untyped(), 0, {});

                    result.emplace_back(PropInfoInternal{name, std::move(typeExp)});
                    result.emplace_back(PropInfoInternal{encryptedName, std::move(typeExpEnc)});
                }

                break;
            }
            case core::Names::feeRuleProp().rawId(): {
                if (send->numPosArgs > 0) {
                    auto *lit = ast::cast_tree<ast::Literal>(send->args.front());
                    if (lit && lit->isSymbol(ctx)) {
                        core::NameRef propName =
                            ctx.state.enterNameConstant(absl::StrCat(lit->asSymbol(ctx).show(ctx), "_rules"));
                        auto typeExp = ast::MK::Send(send->loc, ast::MK::Constant(send->loc, core::Symbols::T()),
                                                     core::Names::untyped(), 0, {});
                        result.emplace_back(PropInfoInternal{std::move(propName), std::move(typeExp)});
                    }
                }

                break;
            }
            case core::Names::tokenProp().rawId():
            case core::Names::timestampedTokenProp().rawId(): {
                result.emplace_back(PropInfoInternal{
                    core::Names::token(),
                    ast::MK::Constant(send->loc, core::Symbols::String()),
                });

                break;
            }
            case core::Names::createdProp().rawId(): {
                result.emplace_back(
                    PropInfoInternal{core::Names::created(), ast::MK::Constant(send->loc, core::Symbols::Float())});

                break;
            }
            case core::Names::updatedProp().rawId(): {
                result.emplace_back(
                    PropInfoInternal{core::Names::updated(), ast::MK::Constant(send->loc, core::Symbols::Float())});

                break;
            }
            case core::Names::merchantProp().rawId(): {
                result.emplace_back(PropInfoInternal{
                    core::Names::merchant(),
                    ast::MK::Constant(send->loc, core::Symbols::String()),
                });

                break;
            }
            case core::Names::merchantTokenProp().rawId(): {
                result.emplace_back(PropInfoInternal{
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
                });

                break;
            }
            default:
                break;
        }

        return result;
    }

    std::optional<ast::ExpressionPtr> transformTypeForMutator(core::MutableContext ctx, ast::ExpressionPtr &propType) {
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

    ast::ExpressionPtr preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
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

            const auto ancstName = symbolName(ctx, cnst->symbol);
            if (ancstName == OPUS_DB_SHARD_BY_MERCHANT) {
                ancestors.emplace_back(OPUS_DB_SHARD_BY_MERCHANT_BASE);
            } else if (ancstName == OPUS_STORAGE_WAL_WALABLE) {
                ancestors.emplace_back(OPUS_STORAGE_WAL_WALABLE_CLASSMETHODS);
            }
            ancestors.emplace_back(std::move(ancstName));
        }

        const vector<core::NameRef> className = symbolName(ctx, original.symbol);
        nestingScopes.emplace_back(className);
        dslInfo.emplace(className, DSLInfo{{}, ancestors, file, {}, {}});

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0 || !validScope) {
            // Not in any scope
            return tree;
        }

        nestingScopes.pop_back();

        return tree;
    }

    ast::ExpressionPtr preTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
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

            const auto propInfos = parseProp(ctx, original);
            if (!propInfos.empty()) {
                for (const auto &propInfo : propInfos) {
                    std::optional<std::string> typeStr;
                    if (propInfo.typeExp.has_value()) {
                        typeStr = std::move(*(propInfo.typeExp)).toString(ctx);
                    }

                    core::NameRef name = std::move(propInfo.name);
                    dslInfo[curScope].props.emplace_back(PropInfo{name, std::move(typeStr)});
                }
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

            auto &sym = cnst->symbol;
            auto name = (!sym.isClassOrModule() || sym != core::Symbols::StubModule()) ? symbolName(ctx, sym)
                                                                                       : constantName(ctx, *cnst);
            dslInfo[curScope].model = std::move(name);
        }

        return tree;
    }

    ast::ExpressionPtr preTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0 || !validScope) {
            // Not already in a valid scope
            return tree;
        }

        auto &curScope = nestingScopes.back();
        if (curScope == CHALK_ODM_IMMUTABLE_MODEL || curScope == OPUS_DB_SHARD_BY_MERCHANT_BASE ||
            curScope == OPUS_STORAGE_WAL_WALABLE_CLASSMETHODS) {
            return tree;
        }

        validScope = false;
        return tree;
    }

    ast::ExpressionPtr postTransformMethodDef(core::MutableContext ctx, ast::ExpressionPtr tree) {
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

DSLAnalysisFile DSLAnalysis::generate(core::MutableContext ctx, ast::ParsedFile tree, const CRCBuilder &crcBuilder) {
    DSLAnalysisWalk walk(tree.file);
    ast::TreeMap::apply(ctx, walk, move(tree.tree));
    auto daf = walk.dslAnalysisFile();
    auto src = tree.file.data(ctx).source();
    daf.cksum = crcBuilder.crc32(src);
    return daf;
}

} // namespace sorbet::autogen
