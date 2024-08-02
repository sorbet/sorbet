#include "rewriter/Prop.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"

using namespace std;

namespace sorbet::rewriter {
namespace {

// these helpers work on a purely syntactic level. for instance, this function determines if an expression is `T`,
// either with no scope or with the root scope (i.e. `::T`). this might not actually refer to the `T` that we define for
// users, but we don't know that information in the Rewriter passes.
bool isT(const ast::ExpressionPtr &expr) {
    auto *t = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return t != nullptr && t->cnst == core::Names::Constants::T() && ast::MK::isRootScope(t->scope);
}

bool isTNilable(const ast::ExpressionPtr &expr) {
    auto *nilable = ast::cast_tree<ast::Send>(expr);
    return nilable != nullptr && nilable->fun == core::Names::nilable() && isT(nilable->recv);
}

bool isTUntyped(const ast::ExpressionPtr &expr) {
    auto *send = ast::cast_tree<ast::Send>(expr);
    return send != nullptr && send->fun == core::Names::untyped() && isT(send->recv);
}

bool isTNilableTUntyped(const ast::ExpressionPtr &expr) {
    if (!isTNilable(expr)) {
        return false;
    }

    auto &body = ast::cast_tree_nonnull<ast::Send>(expr);
    return body.numPosArgs() == 1 && !body.hasKwArgs() && !body.hasBlock() && isTUntyped(body.getPosArg(0));
}

bool isTStruct(const ast::ExpressionPtr &expr) {
    auto *struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::Struct() && isT(struct_->scope);
}

bool isTInexactStruct(const ast::ExpressionPtr &expr) {
    auto *struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::InexactStruct() && isT(struct_->scope);
}

bool isTImmutableStruct(const ast::ExpressionPtr &expr) {
    auto *struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::ImmutableStruct() && isT(struct_->scope);
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
    bool hasWithoutAccessors = false;
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
            return std::nullopt;
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
        auto *sym = ast::cast_tree<ast::Literal>(send->getPosArg(0));
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
        return std::nullopt;
    }

    if (isTNilableTUntyped(ret.type)) {
        auto loc = ret.type.loc();
        if (auto e = ctx.beginError(loc, core::errors::Rewriter::NilableUntyped)) {
            e.setHeader("`{}` is the same as `{}`", "T.nilable(T.untyped)", "T.untyped");
            e.replaceWith("Use `T.untyped`", ctx.locAt(loc), "T.untyped");

            bool addDefault = true;
            if (rulesTree != nullptr) {
                auto *rules = ast::cast_tree<ast::Hash>(rulesTree);
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
        auto *rules = ast::cast_tree<ast::Hash>(rulesTree);
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::immutable())) {
            ret.isImmutable = true;
        }

        if (ASTUtil::hasHashValue(ctx, *rules, core::Names::withoutAccessors())) {
            ret.hasWithoutAccessors = true;
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
            } else {
                if (auto e = ctx.beginError(val.loc(), core::errors::Rewriter::ComputedBySymbol)) {
                    e.setHeader("Value for `{}` must be a symbol literal", "computed_by");
                }
            }
        }

        auto [fk, foreignTree] = ASTUtil::extractHashValue(ctx, *rules, core::Names::foreign());
        if (foreignTree != nullptr) {
            ret.foreign = move(foreignTree);
            ret.foreignKwLit = move(fk);
            if (auto body = ASTUtil::thunkBody(ctx, ret.foreign)) {
                ret.foreign = std::move(body);
            } else {
                if (auto e = ctx.beginError(ret.foreign.loc(), core::errors::Rewriter::PropForeignStrict)) {
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
    }

    if (ret.default_ == nullptr && isTNilable(ret.type)) {
        ret.default_ = ast::MK::Nil(ret.loc);
    }

    return ret;
}

vector<ast::ExpressionPtr> processProp(core::MutableContext ctx, PropInfo &ret, PropContext propContext) {
    vector<ast::ExpressionPtr> nodes;

    const auto loc = ret.loc;
    const auto locZero = loc.copyWithZeroLength();
    const auto name = ret.name;
    const auto nameLoc = ret.nameLoc;

    const auto getType = ASTUtil::dupType(ret.type);

    const auto computedByMethodName = ret.computedByMethodName;
    const auto computedByMethodNameLoc = ret.computedByMethodNameLoc;
    const auto computedByMethodNameLocZero = computedByMethodNameLoc.copyWithZeroLength();

    auto ivarName = name.addAt(ctx);

    nodes.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(getType)));

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
    } else if (ret.ifunset == nullptr) {
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
    if (!ret.isImmutable) {
        auto setType = ASTUtil::dupType(ret.type);
        ast::Send::ARGS_store sigArgs;
        sigArgs.emplace_back(ast::MK::Symbol(nameLoc, core::Names::arg0()));
        sigArgs.emplace_back(ASTUtil::dupType(setType));
        nodes.emplace_back(ast::MK::Sig(loc, std::move(sigArgs), ASTUtil::dupType(setType)));

        if (ret.enum_ == nullptr) {
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
    if (ret.foreign) {
        ast::ExpressionPtr type;
        ast::ExpressionPtr nonNilType;
        if (ASTUtil::dupType(ret.foreign) == nullptr) {
            // If it's not a valid type, just use untyped
            type = ast::MK::Untyped(loc);
            nonNilType = ast::MK::Untyped(loc);
        } else {
            type = ast::MK::Nilable(loc, ASTUtil::dupType(ret.foreign));
            nonNilType = ASTUtil::dupType(ret.foreign);
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
        if (ret.foreignKwLit != nullptr) {
            methodLoc = ret.foreignKwLit.loc();
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

ast::ExpressionPtr ensureWithoutAccessors(const PropInfo &prop, const ast::Send *send) {
    ast::ExpressionPtr result = send->deepCopy();

    if (prop.hasWithoutAccessors) {
        return result;
    }

    auto withoutAccessors = ast::MK::Symbol(send->loc, core::Names::withoutAccessors());
    auto true_ = ast::MK::True(send->loc);

    auto *copy = ast::cast_tree<ast::Send>(result);
    if (copy->hasKwArgs() || !copy->hasPosArgs()) {
        // append to the inline keyword arguments of the send
        copy->addKwArg(move(withoutAccessors), move(true_));
    } else {
        if (auto *hash = ast::cast_tree<ast::Hash>(copy->getPosArg(copy->numPosArgs() - 1))) {
            hash->keys.emplace_back(move(withoutAccessors));
            hash->values.emplace_back(move(true_));
        } else {
            copy->addKwArg(move(withoutAccessors), move(true_));
        }
    }

    return result;
}

vector<ast::ExpressionPtr> mkTypedInitialize(core::MutableContext ctx, core::LocOffsets klassLoc,
                                             core::LocOffsets klassDeclLoc, const vector<PropInfo> &props) {
    ast::MethodDef::ARGS_store args;
    ast::Send::ARGS_store sigArgs;
    args.reserve(props.size());
    sigArgs.reserve(props.size() * 2);

    // add all the required props first.
    for (const auto &prop : props) {
        if (prop.default_ != nullptr) {
            continue;
        }
        auto loc = prop.loc;
        args.emplace_back(ast::MK::KeywordArg(loc, prop.name));
        sigArgs.emplace_back(ast::MK::Symbol(loc, prop.name));
        sigArgs.emplace_back(prop.type.deepCopy());
    }

    // then, add all the optional props.
    for (const auto &prop : props) {
        if (prop.default_ == nullptr) {
            continue;
        }
        auto loc = prop.loc;
        args.emplace_back(ast::MK::OptionalArg(loc, ast::MK::KeywordArg(loc, prop.name), prop.default_.deepCopy()));
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
    result.emplace_back(
        ast::MK::SyntheticMethod(klassLoc, klassDeclLoc, core::Names::initialize(), std::move(args), std::move(body)));
    return result;
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
    UnorderedMap<void *, vector<ast::ExpressionPtr>> replaceNodes;
    replaceNodes.reserve(klass->rhs.size());
    vector<PropInfo> props;
    for (auto &stat : klass->rhs) {
        auto *send = ast::cast_tree<ast::Send>(stat);
        if (send == nullptr) {
            continue;
        }
        auto propInfo = parseProp(ctx, send);
        if (!propInfo.has_value()) {
            continue;
        }

        if (!propInfo->isImmutable && syntacticSuperClass == SyntacticSuperClass::TImmutableStruct) {
            if (auto e = ctx.beginError(propInfo->loc, core::errors::Rewriter::InvalidStructMember)) {
                e.setHeader("Cannot use `{}` in an immutable struct", "prop");
                e.replaceWith("Use `const`", ctx.locAt(propInfo->loc), "const");
            }
            continue;
        }

        auto processed = processProp(ctx, propInfo.value(), propContext);
        ENFORCE(!processed.empty(), "if parseProp completed successfully, processProp must complete too");

        vector<ast::ExpressionPtr> nodes;
        nodes.emplace_back(ensureWithoutAccessors(propInfo.value(), send));
        nodes.insert(nodes.end(), make_move_iterator(processed.begin()), make_move_iterator(processed.end()));
        replaceNodes[stat.get()] = std::move(nodes);
        props.emplace_back(std::move(propInfo.value()));
    }
    auto oldRHS = std::move(klass->rhs);
    klass->rhs.clear();
    klass->rhs.reserve(oldRHS.size());
    // we define our synthesized initialize first so that if the user wrote one themselves, it overrides ours.
    if (wantTypedInitialize(syntacticSuperClass)) {
        // For direct T::Struct subclasses, we know that seeing no props means the constructor should be zero-arity.
        for (auto &stat : mkTypedInitialize(ctx, klass->loc, klass->declLoc, props)) {
            klass->rhs.emplace_back(std::move(stat));
        }
    }
    // this is cargo-culted from rewriter.cc.
    for (auto &stat : oldRHS) {
        auto replacement = replaceNodes.find(stat.get());
        if (replacement == replaceNodes.end()) {
            klass->rhs.emplace_back(std::move(stat));
        } else {
            for (auto &newNode : replacement->second) {
                klass->rhs.emplace_back(std::move(newNode));
            }
        }
    }
}

}; // namespace sorbet::rewriter
