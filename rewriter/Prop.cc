#include "rewriter/Prop.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rewriter {
namespace {

bool isTNilableTUntyped(const ast::ExpressionPtr &expr) {
    if (!ast::MK::isTNilable(expr)) {
        return false;
    }

    auto &body = ast::cast_tree_nonnull<ast::Send>(expr);
    return body.numPosArgs() == 1 && !body.hasKwArgs() && !body.hasBlock() && ast::MK::isTUntyped(body.getPosArg(0));
}

bool isTStruct(const ast::ExpressionPtr &expr) {
    auto struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::Struct() &&
           ast::MK::isTApproximate(struct_->scope);
}

bool isTInexactStruct(const ast::ExpressionPtr &expr) {
    auto struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::InexactStruct() &&
           ast::MK::isTApproximate(struct_->scope);
}

bool isTImmutableStruct(const ast::ExpressionPtr &expr) {
    auto struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::ImmutableStruct() &&
           ast::MK::isTApproximate(struct_->scope);
}

enum class SyntacticSuperClass {
    Unknown,
    TStruct,
    TInexactStruct,
    TImmutableStruct,
};

bool wantSimpleIVarGet(SyntacticSuperClass syntacticSuperClass) {
    switch (syntacticSuperClass) {
        case SyntacticSuperClass::TStruct:
        case SyntacticSuperClass::TInexactStruct:
        case SyntacticSuperClass::TImmutableStruct:
            return true;
        case SyntacticSuperClass::Unknown:
            return false;
    }
}

bool knownNonDocument(SyntacticSuperClass syntacticSuperClass) {
    switch (syntacticSuperClass) {
        case SyntacticSuperClass::TStruct:
        case SyntacticSuperClass::TInexactStruct:
        case SyntacticSuperClass::TImmutableStruct:
            return true;
        case SyntacticSuperClass::Unknown:
            return false;
    }
}

bool wantTypedInitialize(SyntacticSuperClass syntacticSuperClass) {
    switch (syntacticSuperClass) {
        case SyntacticSuperClass::TStruct:
        case SyntacticSuperClass::TImmutableStruct:
            return true;
        case SyntacticSuperClass::TInexactStruct:
        case SyntacticSuperClass::Unknown:
            return false;
    }
}

struct PropContext {
    SyntacticSuperClass syntacticSuperClass = SyntacticSuperClass::Unknown;
    ast::ClassDef::Kind classDefKind;
};

struct PropInfo {
    core::LocOffsets loc;
    bool isImmutable = false;
    ast::ExpressionPtr getterOverride;
    ast::ExpressionPtr setterOverride;
    core::NameRef name;
    core::LocOffsets nameLoc;
    ast::ExpressionPtr type;
    ast::ExpressionPtr default_;
    core::NameRef computedByMethodName;
    core::LocOffsets computedByMethodNameLoc;
    ast::ExpressionPtr foreignKwLit;
    ast::ExpressionPtr foreign;
    ast::ExpressionPtr enum_;
    ast::ExpressionPtr ifunset;
};

struct NodesAndPropInfo {
    vector<ast::ExpressionPtr> nodes;
    PropInfo propInfo;
};

void emitBadOverride(core::MutableContext ctx, const core::LocOffsets loc, core::NameRef name) {
    if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::PropBadOverride)) {
        e.setHeader("Malformed `{}` in prop `{}`: valid values are `{}`, `{}`, `{}`, or a hash literal", "override",
                    name.show(ctx), "true", ":reader", ":writer");
    }
}

ast::ExpressionPtr elaborateOverride(core::MutableContext ctx, core::LocOffsets overrideLoc, ast::Hash &opts,
                                     core::NameRef propName, core::NameRef key, string_view rendered) {
    auto [_key, arg] = ASTUtil::extractHashValue(ctx, opts, key);
    // no override key present
    if (arg == nullptr) {
        return nullptr;
    } else if (auto lit = ast::cast_tree<ast::Literal>(arg)) {
        if (lit->isTrue(ctx)) {
            return ast::MK::OverrideStrict(overrideLoc);
        } else if (lit->isFalse(ctx)) {
            return nullptr;
        } else if (auto e = ctx.beginIndexerError(lit->loc, core::errors::Rewriter::PropBadOverride)) {
            e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}`, `{}` or `{}`", rendered,
                        propName.show(ctx), "true", "{allow_incompatible: :visibility}", "{allow_incompatible: true}");
        }
    } else if (auto rOpts = ast::cast_tree<ast::Hash>(arg)) {
        auto [allowIncompatKey, cfg] = ASTUtil::extractHashValue(ctx, *rOpts, core::Names::allowIncompatible());
        if (auto clit = ast::cast_tree<ast::Literal>(cfg)) {
            auto allowIncompatLoc = allowIncompatKey.loc();
            if (clit->isTrue(ctx)) {
                auto trueNode = ast::MK::True(clit->loc);
                return ast::MK::OverrideAllowIncompatible(overrideLoc, allowIncompatLoc, std::move(trueNode));
            } else if (clit->isFalse(ctx)) {
                return nullptr;
            } else if (clit->isSymbol()) {
                auto sym = clit->asSymbol();
                if (sym == core::Names::visibility()) {
                    auto visibilityNode = ast::MK::Symbol(clit->loc, core::Names::visibility());
                    return ast::MK::OverrideAllowIncompatible(overrideLoc, allowIncompatLoc, std::move(visibilityNode));
                } else if (auto e = ctx.beginIndexerError(clit->loc, core::errors::Rewriter::PropBadOverride)) {
                    e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}`, `{}` or `{}`", rendered,
                                propName.show(ctx), "true", "{allow_incompatible: :visibility}",
                                "{allow_incompatible: true}");
                }
            } else if (auto e = ctx.beginIndexerError(clit->loc, core::errors::Rewriter::PropBadOverride)) {
                e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}`, `{}` or `{}`", rendered,
                            propName.show(ctx), "true", "{allow_incompatible: :visibility}",
                            "{allow_incompatible: true}");
            }
        } else if (cfg != nullptr) {
            if (auto e = ctx.beginIndexerError(cfg.loc(), core::errors::Rewriter::PropBadOverride)) {
                e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}`, `{}` or `{}`", rendered,
                            propName.show(ctx), "true", "{allow_incompatible: :visibility}",
                            "{allow_incompatible: true}");
            }
        } else if (auto e = ctx.beginIndexerError(rOpts->loc, core::errors::Rewriter::PropBadOverride)) {
            e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}`, `{}` or `{}`", rendered,
                        propName.show(ctx), "true", "{allow_incompatible: :visibility}", "{allow_incompatible: true}");
        }
    } else {
        if (auto e = ctx.beginIndexerError(arg.loc(), core::errors::Rewriter::PropBadOverride)) {
            e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}`, `{}` or `{}`", rendered,
                        propName.show(ctx), "true", "{allow_incompatible: :visibility}", "{allow_incompatible: true}");
        }
    }
    return nullptr;
}

optional<PropInfo> parseProp(core::MutableContext ctx, const ast::Send *send) {
    PropInfo ret;
    ret.loc = send->loc;

    // ----- Is this a send we care about? -----
    switch (send->fun.rawId()) {
        case core::Names::prop().rawId():
            // Nothing special
            break;
        case core::Names::const_().rawId():
            ret.isImmutable = true;
            break;
        case core::Names::tokenProp().rawId():
        case core::Names::timestampedTokenProp().rawId(): {
            ret.name = core::Names::token();
            auto beginPos = send->loc.beginPos() + (send->fun == core::Names::timestampedTokenProp() ? 12 : 0);
            ret.nameLoc = core::LocOffsets{beginPos, beginPos + 5}; // get the 'token' part of it
            ret.type = ast::MK::Constant(send->loc, core::Symbols::String());
            break;
        }
        case core::Names::createdProp().rawId():
            ret.name = core::Names::created();
            // 5 is the length of the _prop suffix
            ret.nameLoc = core::LocOffsets{send->loc.beginPos(), send->loc.endPos() - 5};
            ret.type = ast::MK::Constant(send->loc, core::Symbols::Float());
            break;
        case core::Names::updatedProp().rawId(): {
            ret.name = send->fun == core::Names::createdProp() ? core::Names::created() : core::Names::updated();
            // 5 is the length of the _prop suffix
            ret.nameLoc = core::LocOffsets{send->loc.beginPos(), send->loc.endPos() - 5};
            ret.type = ASTUtil::mkNilable(send->loc, ast::MK::UnresolvedConstant(send->loc, ast::MK::EmptyTree(),
                                                                                 core::Names::Constants::Numeric()));
            break;
        }
        case core::Names::merchantProp().rawId():
            ret.isImmutable = true;
            // This is a lie; technically the API allows an optional `name:` keyword arg to
            // customize the method name. It defaults to `:merchant`
            ret.name = core::Names::merchant();
            // 5 is the length of the _prop suffix
            ret.nameLoc = core::LocOffsets{send->loc.beginPos(), send->loc.endPos() - 5};
            ret.type = ast::MK::Constant(send->loc, core::Symbols::String());
            ret.foreign = ast::MK::UnresolvedConstant(
                send->loc,
                ast::MK::UnresolvedConstant(
                    send->loc,
                    ast::MK::UnresolvedConstant(
                        send->loc,
                        ast::MK::UnresolvedConstant(send->loc, ast::MK::EmptyTree(), core::Names::Constants::Opus()),
                        core::Names::Constants::Account()),
                    core::Names::Constants::Model()),
                core::Names::Constants::Merchant());
            break;
        case core::Names::merchantTokenProp().rawId():
            ret.isImmutable = true;
            // This is a lie; technically the API allows an optional `merchant:` keyword arg to
            // customize the method name. It defaults to `:merchant`
            ret.name = core::Names::merchant();
            // 5 is the length of the _prop suffix
            ret.nameLoc = core::LocOffsets{send->loc.beginPos(), send->loc.endPos() - 5};

            ret.type = ast::MK::UnresolvedConstant(
                send->loc,
                ast::MK::UnresolvedConstant(
                    send->loc,
                    ast::MK::UnresolvedConstant(
                        send->loc,
                        ast::MK::UnresolvedConstant(send->loc,
                                                    ast::MK::UnresolvedConstant(send->loc, ast::MK::EmptyTree(),
                                                                                core::Names::Constants::Opus()),
                                                    core::Names::Constants::Autogen()),
                        core::Names::Constants::Tokens()),
                    core::Names::Constants::AccountModelMerchant()),
                core::Names::Constants::Token());
            break;

        default:
            return nullopt;
    }

    auto expectedPosArgs = 3;
    if (send->hasKwArgs()) {
        expectedPosArgs = 2;
    }

    if (send->numPosArgs() > expectedPosArgs) {
        // Too many args, even if all optional args were provided.
        return nullopt;
    }

    // ----- What's the prop's name? -----
    if (!ret.name.exists()) {
        if (!send->hasPosArgs()) {
            return nullopt;
        }
        auto sym = ast::cast_tree<ast::Literal>(send->getPosArg(0));
        if (!sym || !sym->isSymbol()) {
            return nullopt;
        }
        ret.name = sym->asSymbol();
        ENFORCE(ctx.locAt(sym->loc).exists());
        ENFORCE(!ctx.locAt(sym->loc).source(ctx).value().empty() && ctx.locAt(sym->loc).source(ctx).value()[0] == ':');
        ret.nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};
        const auto nameValue = ctx.locAt(ret.nameLoc).source(ctx).value();
        if ((nameValue.front() == '\'' && nameValue.back() == '\'') ||
            (nameValue.front() == '\"' && nameValue.back() == '\"')) {
            ret.nameLoc = core::LocOffsets{ret.nameLoc.beginPos() + 1, ret.nameLoc.endPos() - 1};
        }
    }

    // ----- What's the prop's type? -----
    if (ret.type == nullptr) {
        if (send->numPosArgs() == 1) {
            // Type must have been inferred from prop method (like created_prop) or
            // been given in second argument.
            return nullopt;
        }

        ret.type = ASTUtil::dupType(send->getPosArg(1));
        if (ret.type == nullptr) {
            return nullopt;
        }
    }

    ENFORCE(ASTUtil::dupType(ret.type) != nullptr, "No obvious type AST for this prop");

    // ----- Does the prop have any extra options? -----

    // Deep copy the rules hash so that we can destruct it at will to parse things,
    // without having to worry about whether we stole things from the tree.
    ast::ExpressionPtr rulesTree = ASTUtil::mkKwArgsHash(send);
    if (rulesTree == nullptr && send->numPosArgs() >= expectedPosArgs) {
        // No rules, but 3 args including name and type. Also not a T::Props
        return nullopt;
    }

    if (isTNilableTUntyped(ret.type)) {
        auto loc = ret.type.loc();
        if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::NilableUntyped)) {
            e.setHeader("`{}` is the same as `{}`", "T.nilable(T.untyped)", "T.untyped");
            e.replaceWith("Use `T.untyped`", ctx.locAt(loc), "T.untyped");

            bool addDefault = true;
            if (rulesTree != nullptr) {
                auto rules = ast::cast_tree<ast::Hash>(rulesTree);
                addDefault = !ASTUtil::hasHashValue(ctx, *rules, core::Names::default_());
            }

            if (addDefault) {
                auto end = core::Loc{ctx.file, send->loc.copyEndWithZeroLength()};
                e.replaceWith("Add `default: nil`", end, ", default: nil");
            }
        }

        // rewrite the type to T.untyped to avoid re-raising the same error later on.
        ret.type = ast::MK::Untyped(loc);
    }
    // ----- Parse any extra options -----
    if (rulesTree) {
        auto rules = ast::cast_tree<ast::Hash>(rulesTree);
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::immutable())) {
            ret.isImmutable = true;
        }

        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::factory())) {
            ret.default_ = ast::MK::RaiseTypedUnimplemented(ret.loc);
        } else if (ASTUtil::hasHashValue(ctx, *rules, core::Names::default_())) {
            auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::default_());
            ret.default_ = std::move(val);
        }

        // e.g. `const :foo, type, computed_by: :method_name`
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::computedBy())) {
            auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::computedBy());
            auto lit = ast::cast_tree<ast::Literal>(val);
            if (lit != nullptr && lit->isSymbol()) {
                ret.computedByMethodNameLoc = lit->loc;
                ret.computedByMethodName = lit->asSymbol();
            } else if (auto e = ctx.beginIndexerError(val.loc(), core::errors::Rewriter::ComputedBySymbol)) {
                e.setHeader("Value for `{}` must be a symbol literal", "computed_by");
            }
        }

        auto [fk, foreignTree] = ASTUtil::extractHashValue(ctx, *rules, core::Names::foreign());
        if (foreignTree != nullptr) {
            ret.foreign = move(foreignTree);
            ret.foreignKwLit = move(fk);
            if (auto body = ASTUtil::thunkBody(ctx, ret.foreign)) {
                ret.foreign = std::move(body);
            } else {
                if (auto e = ctx.beginIndexerError(ret.foreign.loc(), core::errors::Rewriter::PropForeignStrict)) {
                    e.setHeader("The argument to `{}` must be a lambda", "foreign:");
                    auto foreignLoc = core::Loc{ctx.file, ret.foreign.loc()};
                    if (auto foreignSource = foreignLoc.source(ctx)) {
                        e.replaceWith("Convert to lambda", foreignLoc, "-> {{ {} }}", foreignSource.value());
                    }
                }
            }
        }

        auto [enumKey, enum_] = ASTUtil::extractHashValue(ctx, *rules, core::Names::enum_());
        if (enum_ != nullptr) {
            ret.enum_ = std::move(enum_);
        }

        auto [ifunsetKey, ifunset] = ASTUtil::extractHashValue(ctx, *rules, core::Names::ifunset());
        if (ifunset != nullptr) {
            ret.ifunset = std::move(ifunset);
        }

        if (send->fun == core::Names::merchantTokenProp()) {
            auto [_nameKey, nameValue] = ASTUtil::extractHashValue(ctx, *rules, core::Names::name());
            if (nameValue != nullptr) {
                if (auto lit = ast::cast_tree<ast::Literal>(nameValue)) {
                    if (lit->isSymbol()) {
                        ret.name = lit->asSymbol();
                        ret.nameLoc = nameValue.loc();
                    }
                }
            }
        }
        auto [overrideKey, overrideArg] = ASTUtil::extractHashValue(ctx, *rules, core::Names::override_());
        ENFORCE(ret.name.exists());
        if (overrideArg != nullptr) {
            auto overrideLoc = overrideKey.loc();
            if (auto lit = ast::cast_tree<ast::Literal>(overrideArg)) {
                if (lit->isTrue(ctx)) {
                    ret.getterOverride = ast::MK::OverrideStrict(overrideLoc);
                    ret.setterOverride = ast::MK::OverrideStrict(overrideLoc);
                } else if (lit->isFalse(ctx)) {
                    ret.getterOverride = nullptr;
                    ret.setterOverride = nullptr;
                } else if (lit->isSymbol()) {
                    auto sym = lit->asSymbol();
                    if (sym == core::Names::reader()) {
                        ret.getterOverride = ast::MK::OverrideStrict(overrideLoc);
                    } else if (sym == core::Names::writer()) {
                        ret.setterOverride = ast::MK::OverrideStrict(overrideLoc);
                    } else {
                        emitBadOverride(ctx, lit->loc, ret.name);
                    }
                } else {
                    emitBadOverride(ctx, lit->loc, ret.name);
                }
            } else if (auto opts = ast::cast_tree<ast::Hash>(overrideArg)) {
                auto [allowIncompatKey, allowIncompatArg] =
                    ASTUtil::extractHashValue(ctx, *opts, core::Names::allowIncompatible());
                if (allowIncompatArg != nullptr) {
                    auto allowIncompatLoc = allowIncompatKey.loc();
                    if (auto lit = ast::cast_tree<ast::Literal>(allowIncompatArg)) {
                        if (lit->isTrue(ctx)) {
                            auto trueNodeGetter = ast::MK::True(lit->loc);
                            auto trueNodeSetter = ast::MK::True(lit->loc);
                            ret.getterOverride = ast::MK::OverrideAllowIncompatible(overrideLoc, allowIncompatLoc,
                                                                                    std::move(trueNodeGetter));
                            ret.setterOverride = ast::MK::OverrideAllowIncompatible(overrideLoc, allowIncompatLoc,
                                                                                    std::move(trueNodeSetter));
                        } else if (lit->isFalse(ctx)) {
                            ret.getterOverride = ast::MK::OverrideStrict(overrideLoc);
                            ret.setterOverride = ast::MK::OverrideStrict(overrideLoc);
                        } else if (lit->isSymbol() && lit->asSymbol() == core::Names::visibility()) {
                            auto visibilityNodeGetter = ast::MK::Symbol(lit->loc, core::Names::visibility());
                            auto visibilityNodeSetter = ast::MK::Symbol(lit->loc, core::Names::visibility());
                            ret.getterOverride = ast::MK::OverrideAllowIncompatible(overrideLoc, allowIncompatLoc,
                                                                                    std::move(visibilityNodeGetter));
                            ret.setterOverride = ast::MK::OverrideAllowIncompatible(overrideLoc, allowIncompatLoc,
                                                                                    std::move(visibilityNodeSetter));
                        } else if (auto e = ctx.beginIndexerError(lit->loc, core::errors::Rewriter::PropBadOverride)) {
                            e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}` or `{}`",
                                        "allow_incompatible", ret.name.show(ctx), "true", ":visibility");
                        }
                    } else if (auto e = ctx.beginIndexerError(allowIncompatArg.loc(),
                                                              core::errors::Rewriter::PropBadOverride)) {
                        e.setHeader("Malformed `{}` in override for prop `{}`: expected `{}` or `{}`",
                                    "allow_incompatible", ret.name.show(ctx), "true", ":visibility");
                    }
                } else {
                    ret.getterOverride =
                        elaborateOverride(ctx, overrideLoc, *opts, ret.name, core::Names::reader(), "reader");
                    ret.setterOverride =
                        elaborateOverride(ctx, overrideLoc, *opts, ret.name, core::Names::writer(), "writer");
                }
            } else {
                emitBadOverride(ctx, overrideArg.loc(), ret.name);
            }
        }
    }

    if (ret.default_ == nullptr && ast::MK::isTNilable(ret.type)) {
        ret.default_ = ast::MK::Nil(ret.loc);
    }

    return ret;
}

vector<ast::ExpressionPtr> processProp(core::MutableContext ctx, PropInfo &prop, PropContext propContext) {
    vector<ast::ExpressionPtr> nodes;

    const auto loc = prop.loc;
    const auto locZero = loc.copyWithZeroLength();
    const auto name = prop.name;
    const auto nameLoc = prop.nameLoc;

    const auto getType = ASTUtil::dupType(prop.type);

    const auto computedByMethodName = prop.computedByMethodName;
    const auto computedByMethodNameLoc = prop.computedByMethodNameLoc;
    const auto computedByMethodNameLocZero = computedByMethodNameLoc.copyWithZeroLength();

    auto ivarName = name.addAt(ctx);

    auto readerSig = ast::MK::Sig0(loc, ASTUtil::dupType(getType), move(prop.getterOverride));
    nodes.emplace_back(std::move(readerSig));

    // Generate a real prop body for computed_by: props so Sorbet can assert the
    // existence of the computed_by: method.
    if (computedByMethodName.exists()) {
        // Given `const :foo, type, computed_by: <name>`, where <name> is a Symbol pointing to a class method,
        // assert that the method takes 1 argument (of any type), and returns the same type as the prop,
        // via `T.assert_type!(self.class.compute_foo(T.unsafe(nil)), type)` in the getter.
        auto selfSendClass = ast::MK::Send0(computedByMethodNameLoc, ast::MK::Self(loc), core::Names::class_(),
                                            computedByMethodNameLocZero);
        auto raiseUnimplemented = ast::MK::RaiseUnimplemented(computedByMethodNameLoc);
        auto sendComputedMethod =
            ast::MK::Send1(computedByMethodNameLoc, std::move(selfSendClass), computedByMethodName,
                           computedByMethodNameLocZero, std::move(raiseUnimplemented));
        auto assertTypeMatches =
            ast::MK::AssertType(computedByMethodNameLoc, std::move(sendComputedMethod), ASTUtil::dupType(getType));
        auto insSeq = ast::MK::InsSeq1(loc, std::move(assertTypeMatches), ast::MK::RaiseTypedUnimplemented(loc));
        nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, std::move(insSeq)));
    } else if (prop.ifunset == nullptr) {
        if (wantSimpleIVarGet(propContext.syntacticSuperClass)) {
            ast::MethodDef::Flags flags;
            if (wantTypedInitialize(propContext.syntacticSuperClass)) {
                nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::Instance(nameLoc, ivarName), flags));
            } else {
                // Need to hide the instance variable access, because there wasn't a typed constructor to declare it
                auto ivarGet = ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::instanceVariableGet(), locZero,
                                              ast::MK::Symbol(nameLoc, ivarName));
                nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, std::move(ivarGet), flags));
            }
        } else {
            nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::RaiseTypedUnimplemented(loc)));
        }
    } else {
        nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::RaiseTypedUnimplemented(loc)));
    }

    core::NameRef setName = name.addEq(ctx);

    // Compute the setter
    if (!prop.isImmutable) {
        auto setType = ASTUtil::dupType(prop.type);
        ast::Send::ARGS_store sigArgs;
        sigArgs.emplace_back(ast::MK::Symbol(nameLoc, core::Names::arg0()));
        sigArgs.emplace_back(ASTUtil::dupType(setType));

        auto writerSig = ast::MK::Sig(loc, std::move(sigArgs), ASTUtil::dupType(setType), move(prop.setterOverride));
        nodes.emplace_back(std::move(writerSig));

        if (prop.enum_ == nullptr) {
            if (knownNonDocument(propContext.syntacticSuperClass)) {
                if (wantTypedInitialize(propContext.syntacticSuperClass)) {
                    auto ivarSet = ast::MK::Assign(loc, ast::MK::Instance(nameLoc, ivarName),
                                                   ast::MK::Local(nameLoc, core::Names::arg0()));
                    nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, std::move(ivarSet)));
                } else {
                    // need to hide the instance variable access, because there wasn't a typed constructor to declare it
                    auto ivarSet = ast::MK::Send2(loc, ast::MK::Self(loc), core::Names::instanceVariableSet(), locZero,
                                                  ast::MK::Symbol(nameLoc, ivarName),
                                                  ast::MK::Local(nameLoc, core::Names::arg0()));
                    nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, std::move(ivarSet)));
                }
            } else {
                nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, ast::MK::RaiseTypedUnimplemented(loc)));
            }
        } else {
            nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, ast::MK::RaiseTypedUnimplemented(loc)));
        }
    }

    // Compute the `_` foreign accessor
    if (prop.foreign) {
        ast::ExpressionPtr type;
        ast::ExpressionPtr nonNilType;
        if (ASTUtil::dupType(prop.foreign) == nullptr) {
            // If it's not a valid type, just use untyped
            type = ast::MK::Untyped(loc);
            nonNilType = ast::MK::Untyped(loc);
        } else {
            type = ast::MK::Nilable(loc, ASTUtil::dupType(prop.foreign));
            nonNilType = ASTUtil::dupType(prop.foreign);
        }

        // sig {params(allow_direct_mutation: T.nilable(T::Boolean)).returns(T.nilable($foreign))}
        nodes.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::allowDirectMutation()),
                                         ast::MK::Nilable(loc, ast::MK::T_Boolean(loc)), std::move(type)));

        // def $fk_method(allow_direct_mutation: nil)
        //  T.unsafe(nil)
        // end

        auto fkMethod = ctx.state.enterNameUTF8(name.show(ctx) + "_");

        auto arg = ast::MK::KeywordArgWithDefault(nameLoc, core::Names::allowDirectMutation(), ast::MK::Nil(loc));
        ast::MethodDef::Flags fkFlags;
        fkFlags.discardDef = true;

        core::LocOffsets methodLoc;
        if (prop.foreignKwLit != nullptr) {
            methodLoc = prop.foreignKwLit.loc();
        } else {
            methodLoc = loc;
        }

        auto fkMethodDef = ast::MK::SyntheticMethod1(loc, methodLoc, fkMethod, std::move(arg),
                                                     ast::MK::RaiseTypedUnimplemented(loc), fkFlags);
        nodes.emplace_back(std::move(fkMethodDef));

        // sig {params(opts: T.untyped).returns($foreign)}
        nodes.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::allowDirectMutation()),
                                         ast::MK::Nilable(loc, ast::MK::T_Boolean(loc)), std::move(nonNilType)));

        // def $fk_method_!(**opts)
        //  T.unsafe(nil)
        // end

        auto fkMethodBang = ctx.state.enterNameUTF8(name.show(ctx) + "_!");
        auto arg2 = ast::MK::KeywordArgWithDefault(nameLoc, core::Names::allowDirectMutation(), ast::MK::Nil(loc));
        ast::MethodDef::Flags fkBangFlags;
        fkBangFlags.discardDef = true;
        auto fkMethodDefBang = ast::MK::SyntheticMethod1(loc, loc, fkMethodBang, std::move(arg2),
                                                         ast::MK::RaiseTypedUnimplemented(loc), fkBangFlags);
        nodes.emplace_back(std::move(fkMethodDefBang));
    }

    return nodes;
}

vector<ast::ExpressionPtr> mkTypedInitialize(core::MutableContext ctx, core::LocOffsets klassLoc,
                                             core::LocOffsets klassDeclLoc, const vector<PropInfo> &props) {
    ast::MethodDef::PARAMS_store params;
    ast::Send::ARGS_store sigArgs;
    params.reserve(props.size());
    sigArgs.reserve(props.size() * 2);

    // add all the required props first.
    for (const auto &prop : props) {
        if (prop.default_ != nullptr) {
            continue;
        }
        auto loc = prop.loc;
        params.emplace_back(ast::MK::KeywordArg(loc, prop.name));
        sigArgs.emplace_back(ast::MK::Symbol(loc, prop.name));
        sigArgs.emplace_back(prop.type.deepCopy());
    }

    // then, add all the optional props.
    for (const auto &prop : props) {
        if (prop.default_ == nullptr) {
            continue;
        }
        auto loc = prop.loc;
        params.emplace_back(ast::MK::OptionalParam(loc, ast::MK::KeywordArg(loc, prop.name), prop.default_.deepCopy()));
        sigArgs.emplace_back(ast::MK::Symbol(loc, prop.name));
        sigArgs.emplace_back(prop.type.deepCopy());
    }

    // then initialize all the instance variables in the body
    ast::InsSeq::STATS_store stats;
    for (const auto &prop : props) {
        auto ivarName = prop.name.addAt(ctx);
        stats.emplace_back(ast::MK::Assign(prop.loc, ast::MK::Instance(prop.nameLoc, ivarName),
                                           ast::MK::Local(prop.nameLoc, prop.name)));
    }
    auto body = ast::MK::InsSeq(klassLoc, std::move(stats), ast::MK::Nil(klassDeclLoc));

    vector<ast::ExpressionPtr> result;
    result.emplace_back(ast::MK::SigVoid(klassDeclLoc, std::move(sigArgs)));
    result.emplace_back(ast::MK::SyntheticMethod(klassLoc, klassDeclLoc, core::Names::initialize(), std::move(params),
                                                 std::move(body)));
    return result;
}

vector<ast::ExpressionPtr> runOneStat(core::MutableContext ctx, const PropContext &propContext, vector<PropInfo> &props,
                                      UnorderedMap<core::NameRef, uint32_t> &seenProps,
                                      const ast::ExpressionPtr &stat) {
    auto send = ast::cast_tree<ast::Send>(stat);
    if (send == nullptr) {
        return {};
    }
    auto propInfo = parseProp(ctx, send);
    if (!propInfo.has_value()) {
        return {};
    }

    auto syntacticSuperClass = propContext.syntacticSuperClass;
    if (!propInfo->isImmutable && syntacticSuperClass == SyntacticSuperClass::TImmutableStruct) {
        if (auto e = ctx.beginIndexerError(propInfo->loc, core::errors::Rewriter::InvalidStructMember)) {
            e.setHeader("Cannot use `{}` in an immutable struct", "prop");
            e.replaceWith("Use `const`", ctx.locAt(propInfo->loc), "const");
        }
        return {};
    }

    if (wantTypedInitialize(syntacticSuperClass)) {
        auto it = seenProps.find(propInfo->name);
        if (it != seenProps.end()) {
            if (auto e = ctx.beginIndexerError(propInfo->loc, core::errors::Rewriter::DuplicateProp)) {
                auto headerProp = fmt::format("{} {}", propInfo->isImmutable ? "const" : "prop",
                                              propInfo->name.showAsSymbolLiteral(ctx));
                e.setHeader("The `{}` is defined multiple times", headerProp);
                e.addErrorLine(ctx.locAt(props[it->second].loc), "Originally defined here");
            }
            return {};
        }
    }

    auto processed = processProp(ctx, propInfo.value(), propContext);
    ENFORCE(!processed.empty(), "if parseProp completed successfully, processProp must complete too");

    seenProps[propInfo->name] = props.size();
    props.emplace_back(std::move(propInfo.value()));

    return processed;
}

} // namespace

void Prop::run(core::MutableContext ctx, ast::ClassDef *klass) {
    auto syntacticSuperClass = SyntacticSuperClass::Unknown;
    if (!klass->ancestors.empty()) {
        auto &superClass = klass->ancestors[0];
        if (isTStruct(superClass)) {
            syntacticSuperClass = SyntacticSuperClass::TStruct;
        } else if (isTInexactStruct(superClass)) {
            syntacticSuperClass = SyntacticSuperClass::TInexactStruct;
        } else if (isTImmutableStruct(superClass)) {
            syntacticSuperClass = SyntacticSuperClass::TImmutableStruct;
        }
    }
    auto propContext = PropContext{syntacticSuperClass, klass->kind};
    UnorderedMap<void *, vector<ast::ExpressionPtr>> insertNodesAfterStat;
    insertNodesAfterStat.reserve(klass->rhs.size());
    vector<PropInfo> props;
    UnorderedMap<core::NameRef, uint32_t> seenProps;

    for (const auto &stat : klass->rhs) {
        // This is in a helper function to avoid taking mutable access to `klass->rhs`, to ensure we
        // don't mutate it while we iterate.
        insertNodesAfterStat[stat.get()] = runOneStat(ctx, propContext, props, seenProps, stat);
    }

    vector<ast::ExpressionPtr> typedInitializeStats;
    if (wantTypedInitialize(syntacticSuperClass)) {
        // For direct T::Struct subclasses, we know that seeing no props means the constructor should be zero-arity.
        typedInitializeStats = mkTypedInitialize(ctx, klass->loc, klass->declLoc, props);
    }

    auto capacity = klass->rhs.size() + typedInitializeStats.size();
    for (const auto &[_oldStat, newStats] : insertNodesAfterStat) {
        capacity += newStats.size();
    }

    auto oldRHS = std::move(klass->rhs);
    klass->rhs.clear();
    klass->rhs.reserve(capacity);

    // we define our synthesized initialize first so that if the user wrote one themselves, it redefines ours.
    for (auto &stat : typedInitializeStats) {
        klass->rhs.emplace_back(std::move(stat));
    }

    for (auto &stat : oldRHS) {
        auto newNodes = insertNodesAfterStat.find(stat.get());
        klass->rhs.emplace_back(std::move(stat));
        if (newNodes != insertNodesAfterStat.end()) {
            for (auto &newNode : newNodes->second) {
                klass->rhs.emplace_back(std::move(newNode));
            }
        }
    }
}

}; // namespace sorbet::rewriter
