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
bool isT(ast::TreePtr &expr) {
    auto *t = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return t != nullptr && t->cnst == core::Names::Constants::T() && ast::MK::isRootScope(t->scope);
}

bool isTNilable(ast::TreePtr &expr) {
    auto *nilable = ast::cast_tree<ast::Send>(expr);
    return nilable != nullptr && nilable->fun == core::Names::nilable() && isT(nilable->recv);
}

bool isTStruct(ast::TreePtr &expr) {
    auto *struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::Struct() && isT(struct_->scope);
}

bool isTInexactStruct(ast::TreePtr &expr) {
    auto *struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::InexactStruct() && isT(struct_->scope);
}

bool isChalkODMDocument(ast::TreePtr &expr) {
    auto *document = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (document == nullptr || document->cnst != core::Names::Constants::Document()) {
        return false;
    }
    auto *odm = ast::cast_tree<ast::UnresolvedConstantLit>(document->scope);
    if (odm == nullptr || odm->cnst != core::Names::Constants::ODM()) {
        return false;
    }
    auto *chalk = ast::cast_tree<ast::UnresolvedConstantLit>(odm->scope);
    return chalk != nullptr && chalk->cnst == core::Names::Constants::Chalk() && ast::MK::isRootScope(chalk->scope);
}

enum class SyntacticSuperClass {
    Unknown,
    TStruct,
    TInexactStruct,
    ChalkODMDocument,
};

bool knownNonModel(SyntacticSuperClass syntacticSuperClass) {
    switch (syntacticSuperClass) {
        case SyntacticSuperClass::TStruct:
        case SyntacticSuperClass::TInexactStruct:
        case SyntacticSuperClass::ChalkODMDocument:
            return true;
        case SyntacticSuperClass::Unknown:
            return false;
    }
}

bool knownNonDocument(SyntacticSuperClass syntacticSuperClass) {
    switch (syntacticSuperClass) {
        case SyntacticSuperClass::TStruct:
        case SyntacticSuperClass::TInexactStruct:
            return true;
        case SyntacticSuperClass::ChalkODMDocument:
        case SyntacticSuperClass::Unknown:
            return false;
    }
}

bool wantTypedInitialize(SyntacticSuperClass syntacticSuperClass) {
    switch (syntacticSuperClass) {
        case SyntacticSuperClass::TStruct:
            return true;
        case SyntacticSuperClass::TInexactStruct:
        case SyntacticSuperClass::ChalkODMDocument:
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
    ast::TreePtr type;
    ast::TreePtr default_;
    core::NameRef computedByMethodName;
    core::LocOffsets computedByMethodNameLoc;
    ast::TreePtr foreign;
    ast::TreePtr enum_;
    ast::TreePtr ifunset;
};

struct NodesAndPropInfo {
    vector<ast::TreePtr> nodes;
    PropInfo propInfo;
};

optional<PropInfo> parseProp(core::MutableContext ctx, const ast::Send *send) {
    PropInfo ret;
    ret.loc = send->loc;

    // ----- Is this a send we care about? -----
    switch (send->fun._id) {
        case core::Names::prop()._id:
            // Nothing special
            break;
        case core::Names::const_()._id:
            ret.isImmutable = true;
            break;
        case core::Names::tokenProp()._id:
        case core::Names::timestampedTokenProp()._id:
            ret.name = core::Names::token();
            ret.nameLoc = core::LocOffsets{send->loc.beginPos() +
                                               (send->fun._id == core::Names::timestampedTokenProp()._id ? 12 : 0),
                                           send->loc.endPos() - 5}; // get the 'token' part of it
            ret.type = ast::MK::Constant(send->loc, core::Symbols::String());
            break;
        case core::Names::createdProp()._id:
            ret.name = core::Names::created();
            ret.nameLoc =
                core::LocOffsets{send->loc.beginPos(),
                                 send->loc.endPos() - 5}; // 5 is the difference between `created_prop` and `created`
            ret.type = ast::MK::Constant(send->loc, core::Symbols::Float());
            break;
        case core::Names::merchantProp()._id:
            ret.isImmutable = true;
            ret.name = core::Names::merchant();
            ret.nameLoc =
                core::LocOffsets{send->loc.beginPos(),
                                 send->loc.endPos() - 5}; // 5 is the difference between `merchant_prop` and `merchant`
            ret.type = ast::MK::Constant(send->loc, core::Symbols::String());
            break;

        default:
            return std::nullopt;
    }

    if (send->args.size() >= 4) {
        // Too many args, even if all optional args were provided.
        return nullopt;
    }

    // ----- What's the prop's name? -----
    if (!ret.name.exists()) {
        if (send->args.empty()) {
            return nullopt;
        }
        auto *sym = ast::cast_tree_const<ast::Literal>(send->args[0]);
        if (!sym || !sym->isSymbol(ctx)) {
            return nullopt;
        }
        ret.name = sym->asSymbol(ctx);
        ENFORCE(!core::Loc(ctx.file, sym->loc).source(ctx).empty() &&
                core::Loc(ctx.file, sym->loc).source(ctx)[0] == ':');
        ret.nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};
    }

    // ----- What's the prop's type? -----
    if (ret.type == nullptr) {
        if (send->args.size() == 1) {
            // Type must have been inferred from prop method (like created_prop) or
            // been given in second argument.
            return nullopt;
        } else {
            ret.type = ASTUtil::dupType(send->args[1]);
            if (ret.type == nullptr) {
                return nullopt;
            }
        }
    }

    ENFORCE(ASTUtil::dupType(ret.type) != nullptr, "No obvious type AST for this prop");

    // ----- Does the prop have any extra options? -----
    ast::TreePtr rulesTree;
    if (!send->args.empty()) {
        if (auto back = ast::cast_tree_const<ast::Hash>(send->args.back())) {
            // Deep copy the rules hash so that we can destruct it at will to parse things,
            // without having to worry about whether we stole things from the tree.
            rulesTree = back->deepCopy();
        }
    }
    if (rulesTree == nullptr && send->args.size() >= 3) {
        // No rules, but 3 args including name and type. Also not a T::Props
        return std::nullopt;
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
            ret.default_ = ast::MK::RaiseUnimplemented(ret.loc);
        } else if (ASTUtil::hasHashValue(ctx, *rules, core::Names::default_())) {
            auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::default_());
            ret.default_ = std::move(val);
        }

        // e.g. `const :foo, type, computed_by: :method_name`
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::computedBy())) {
            auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::computedBy());
            auto lit = ast::cast_tree<ast::Literal>(val);
            if (lit != nullptr && lit->isSymbol(ctx)) {
                ret.computedByMethodNameLoc = lit->loc;
                ret.computedByMethodName = lit->asSymbol(ctx);
            } else {
                if (auto e = ctx.beginError(val->loc, core::errors::Rewriter::ComputedBySymbol)) {
                    e.setHeader("Value for `{}` must be a symbol literal", "computed_by");
                }
            }
        }

        auto [fk, foreignTree] = ASTUtil::extractHashValue(ctx, *rules, core::Names::foreign());
        if (foreignTree != nullptr) {
            ret.foreign = move(foreignTree);
            if (auto body = ASTUtil::thunkBody(ctx, ret.foreign)) {
                ret.foreign = std::move(body);
            } else {
                if (auto e = ctx.beginError(ret.foreign->loc, core::errors::Rewriter::PropForeignStrict)) {
                    e.setHeader("The argument to `{}` must be a lambda", "foreign:");
                    e.replaceWith("Convert to lambda", core::Loc(ctx.file, ret.foreign->loc), "-> {{{}}}",
                                  core::Loc(ctx.file, ret.foreign->loc).source(ctx));
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
    }

    if (ret.default_ == nullptr && isTNilable(ret.type)) {
        ret.default_ = ast::MK::Nil(ret.loc);
    }

    return ret;
}

vector<ast::TreePtr> processProp(core::MutableContext ctx, PropInfo &ret, PropContext propContext) {
    vector<ast::TreePtr> nodes;

    const auto loc = ret.loc;
    const auto name = ret.name;
    const auto nameLoc = ret.nameLoc;

    const auto getType = ASTUtil::dupType(ret.type);

    const auto computedByMethodName = ret.computedByMethodName;
    const auto computedByMethodNameLoc = ret.computedByMethodNameLoc;

    auto ivarName = name.addAt(ctx);

    nodes.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), ASTUtil::dupType(getType)));

    if (computedByMethodName.exists()) {
        // Given `const :foo, type, computed_by: <name>`, where <name> is a Symbol pointing to a class method,
        // assert that the method takes 1 argument (of any type), and returns the same type as the prop,
        // via `T.assert_type!(self.class.compute_foo(T.unsafe(nil)), type)` in the getter.
        auto selfSendClass = ast::MK::Send0(computedByMethodNameLoc, ast::MK::Self(loc), core::Names::class_());
        auto raiseUnimplemented = ast::MK::RaiseUnimplemented(computedByMethodNameLoc);
        auto sendComputedMethod = ast::MK::Send1(computedByMethodNameLoc, std::move(selfSendClass),
                                                 computedByMethodName, std::move(raiseUnimplemented));
        auto assertTypeMatches =
            ast::MK::AssertType(computedByMethodNameLoc, std::move(sendComputedMethod), ASTUtil::dupType(getType));
        auto insSeq = ast::MK::InsSeq1(loc, std::move(assertTypeMatches), ast::MK::RaiseUnimplemented(loc));
        nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, std::move(insSeq)));
    } else if (propContext.classDefKind == ast::ClassDef::Kind::Module) {
        // Not all modules include Kernel, can't make an initialize, etc. so we're punting on props in modules rn.
        nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::RaiseUnimplemented(loc)));
    } else if (ret.ifunset == nullptr) {
        if (knownNonModel(propContext.syntacticSuperClass)) {
            if (wantTypedInitialize(propContext.syntacticSuperClass)) {
                nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::Instance(nameLoc, ivarName)));
            } else {
                // Need to hide the instance variable access, because there wasn't a typed constructor to declare it
                auto ivarGet = ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::instanceVariableGet(),
                                              ast::MK::Symbol(nameLoc, ivarName));
                nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, std::move(ivarGet)));
            }
        } else {
            // Models have a custom decorator, which means we have to forward the prop get to it.
            // If this is actually a T::InexactStruct or Chalk::ODM::Document sub-sub-class, this implementation is
            // correct but does extra work.

            auto arg2 = ast::MK::Local(loc, core::Names::arg2());

            auto ivarGet = ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::instanceVariableGet(),
                                          ast::MK::Symbol(nameLoc, ivarName));
            auto assign = ast::MK::Assign(loc, arg2->deepCopy(), std::move(ivarGet));

            auto class_ = ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::class_());
            auto decorator = ast::MK::Send0(loc, std::move(class_), core::Names::decorator());
            auto propGetLogic = ast::MK::Send3(loc, std::move(decorator), core::Names::propGetLogic(),
                                               ast::MK::Self(loc), ast::MK::Symbol(nameLoc, name), std::move(arg2));

            auto insSeq = ast::MK::InsSeq1(loc, std::move(assign), std::move(propGetLogic));
            nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, std::move(insSeq)));
        }
    } else {
        nodes.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::RaiseUnimplemented(loc)));
    }

    core::NameRef setName = name.addEq(ctx);

    // Compute the setter
    if (!ret.isImmutable) {
        auto setType = ASTUtil::dupType(ret.type);
        nodes.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), ASTUtil::dupType(setType)),
            ASTUtil::dupType(setType)));

        if (propContext.classDefKind == ast::ClassDef::Kind::Module) {
            // Not all modules include Kernel, can't make an initialize, etc. so we're punting on props in modules rn.
            nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, ast::MK::RaiseUnimplemented(loc)));
        } else if (ret.enum_ == nullptr) {
            if (knownNonDocument(propContext.syntacticSuperClass)) {
                if (wantTypedInitialize(propContext.syntacticSuperClass)) {
                    auto ivarSet = ast::MK::Assign(loc, ast::MK::Instance(nameLoc, ivarName),
                                                   ast::MK::Local(nameLoc, core::Names::arg0()));
                    nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, std::move(ivarSet)));
                } else {
                    // need to hide the instance variable access, because there wasn't a typed constructor to declare it
                    auto ivarSet = ast::MK::Send2(loc, ast::MK::Self(loc), core::Names::instanceVariableSet(),
                                                  ast::MK::Symbol(nameLoc, ivarName),
                                                  ast::MK::Local(nameLoc, core::Names::arg0()));
                    nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, std::move(ivarSet)));
                }
            } else {
                // Chalk::ODM::Document classes have special handling for soft freeze
                auto doc = ast::MK::String(loc, core::Names::Chalk_ODM_Document());
                auto nonForcingCnst = ast::MK::Constant(loc, core::Symbols::T_NonForcingConstants());
                auto nonForcingIsA = ast::MK::Send2(loc, std::move(nonForcingCnst), core::Names::nonForcingIsA_p(),
                                                    ast::MK::Self(loc), std::move(doc));
                auto docDecoHelper = ast::MK::Constant(loc, core::Symbols::Chalk_ODM_DocumentDecoratorHelper());
                auto softFreezeLogic = ast::MK::Send2(loc, std::move(docDecoHelper), core::Names::softFreezeLogic(),
                                                      ast::MK::Self(loc), ast::MK::Symbol(loc, name));
                auto softFreezeIf =
                    ast::MK::If(loc, std::move(nonForcingIsA), std::move(softFreezeLogic), ast::MK::EmptyTree());

                // need to hide the instance variable access, because there wasn't a typed constructor to declare it
                auto ivarSet =
                    ast::MK::Send2(loc, ast::MK::Self(loc), core::Names::instanceVariableSet(),
                                   ast::MK::Symbol(nameLoc, ivarName), ast::MK::Local(nameLoc, core::Names::arg0()));
                auto insSeq = ast::MK::InsSeq1(loc, std::move(softFreezeIf), std::move(ivarSet));
                nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, std::move(insSeq)));
            }
        } else {
            nodes.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, ast::MK::RaiseUnimplemented(loc)));
        }
    }

    // Compute the `_` foreign accessor
    if (ret.foreign) {
        ast::TreePtr type;
        ast::TreePtr nonNilType;
        if (ASTUtil::dupType(ret.foreign) == nullptr) {
            // If it's not a valid type, just use untyped
            type = ast::MK::Untyped(loc);
            nonNilType = ast::MK::Untyped(loc);
        } else {
            type = ast::MK::Nilable(loc, ASTUtil::dupType(ret.foreign));
            nonNilType = ASTUtil::dupType(ret.foreign);
        }

        // sig {params(opts: T.untyped).returns(T.nilable($foreign))}
        nodes.emplace_back(
            ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::opts()), ast::MK::Untyped(loc), std::move(type)));

        // def $fk_method(**opts)
        //  T.unsafe(nil)
        // end

        auto fkMethod = ctx.state.enterNameUTF8(name.data(ctx)->show(ctx) + "_");

        ast::TreePtr arg =
            ast::MK::RestArg(nameLoc, ast::MK::KeywordArg(nameLoc, ast::MK::Local(nameLoc, core::Names::opts())));
        nodes.emplace_back(ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), fkMethod, std::move(arg),
                                                     ast::MK::RaiseUnimplemented(loc)));

        // sig {params(opts: T.untyped).returns($foreign)}
        nodes.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::opts()), ast::MK::Untyped(loc),
                                         std::move(nonNilType)));

        // def $fk_method_!(**opts)
        //  T.unsafe(nil)
        // end

        auto fkMethodBang = ctx.state.enterNameUTF8(name.data(ctx)->show(ctx) + "_!");
        ast::TreePtr arg2 =
            ast::MK::RestArg(nameLoc, ast::MK::KeywordArg(nameLoc, ast::MK::Local(nameLoc, core::Names::opts())));
        nodes.emplace_back(ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), fkMethodBang, std::move(arg2),
                                                     ast::MK::RaiseUnimplemented(loc)));
    }

    // Compute the Mutator
    {
        // Compute a setter
        auto setType = ASTUtil::dupType(ret.type);
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), ASTUtil::dupType(setType)),
            ASTUtil::dupType(setType)));
        rhs.emplace_back(ASTUtil::mkSet(ctx, loc, setName, nameLoc, ast::MK::RaiseUnimplemented(loc)));

        // Maybe make a getter
        ast::TreePtr mutator;
        if (ASTUtil::isProbablySymbol(ctx, ret.type, core::Symbols::Hash())) {
            mutator = ASTUtil::mkMutator(ctx, loc, core::Names::Constants::HashMutator());
            auto *send = ast::cast_tree_const<ast::Send>(ret.type);
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 2) {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0]), ASTUtil::dupType(send->args[1]));
            } else {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc),
                                         ast::MK::Untyped(loc));
            }
        } else if (ASTUtil::isProbablySymbol(ctx, ret.type, core::Symbols::Array())) {
            mutator = ASTUtil::mkMutator(ctx, loc, core::Names::Constants::ArrayMutator());
            auto *send = ast::cast_tree_const<ast::Send>(ret.type);
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 1) {
                mutator = ast::MK::Send1(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0]));
            } else {
                mutator = ast::MK::Send1(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc));
            }
        } else if (ast::isa_tree<ast::UnresolvedConstantLit>(ret.type)) {
            // In a perfect world we could know if there was a Mutator we could reference instead, like this:
            // mutator = ast::MK::UnresolvedConstant(loc, ASTUtil::dupType(type.get()),
            // core::Names::Constants::Mutator()); For now we're just going to leave these in method_missing.rbi
        }

        if (mutator.get()) {
            rhs.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(mutator)));
            rhs.emplace_back(ASTUtil::mkGet(ctx, loc, name, ast::MK::RaiseUnimplemented(loc)));

            ast::ClassDef::ANCESTORS_store ancestors;
            auto name = core::Names::Constants::Mutator();
            nodes.emplace_back(ast::MK::Class(loc, core::Loc(ctx.file, loc),
                                              ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), name),
                                              std::move(ancestors), std::move(rhs)));
        }
    }

    return nodes;
}

ast::TreePtr ensureWithoutAccessors(const PropInfo &prop, const ast::Send *send) {
    ast::TreePtr result = send->deepCopy();

    if (prop.hasWithoutAccessors) {
        return result;
    } else {
        auto withoutAccessors = ast::MK::Symbol(send->loc, core::Names::withoutAccessors());
        auto true_ = ast::MK::True(send->loc);

        auto *copy = ast::cast_tree<ast::Send>(result);
        if (copy->args.empty()) {
            copy->args.emplace_back(ast::MK::Hash1(send->loc, std::move(withoutAccessors), std::move(true_)));
        } else if (auto rules = ast::cast_tree<ast::Hash>(copy->args.back())) {
            rules->keys.emplace_back(std::move(withoutAccessors));
            rules->values.emplace_back(std::move(true_));
        } else {
            copy->args.emplace_back(ast::MK::Hash1(send->loc, std::move(withoutAccessors), std::move(true_)));
        }

        return result;
    }
}

vector<ast::TreePtr> mkTypedInitialize(core::MutableContext ctx, core::LocOffsets klassLoc,
                                       const vector<PropInfo> &props) {
    ast::MethodDef::ARGS_store args;
    ast::Hash::ENTRY_store sigKeys;
    ast::Hash::ENTRY_store sigVals;
    args.reserve(props.size());
    sigKeys.reserve(props.size());
    sigVals.reserve(props.size());

    // add all the required props first.
    for (const auto &prop : props) {
        if (prop.default_ != nullptr) {
            continue;
        }
        auto loc = prop.loc;
        args.emplace_back(ast::MK::KeywordArg(loc, ast::MK::Local(loc, prop.name)));
        sigKeys.emplace_back(ast::MK::Symbol(loc, prop.name));
        sigVals.emplace_back(prop.type->deepCopy());
    }

    // then, add all the optional props.
    for (const auto &prop : props) {
        if (prop.default_ == nullptr) {
            continue;
        }
        auto loc = prop.loc;
        args.emplace_back(ast::MK::OptionalArg(loc, ast::MK::KeywordArg(loc, ast::MK::Local(loc, prop.name)),
                                               prop.default_->deepCopy()));
        sigKeys.emplace_back(ast::MK::Symbol(loc, prop.name));
        sigVals.emplace_back(prop.type->deepCopy());
    }

    // then initialize all the instance variables in the body
    ast::InsSeq::STATS_store stats;
    for (const auto &prop : props) {
        auto ivarName = prop.name.addAt(ctx);
        stats.emplace_back(ast::MK::Assign(prop.loc, ast::MK::Instance(prop.nameLoc, ivarName),
                                           ast::MK::Local(prop.nameLoc, prop.name)));
    }
    auto body = ast::MK::InsSeq(klassLoc, std::move(stats), ast::MK::ZSuper(klassLoc));

    vector<ast::TreePtr> result;
    result.emplace_back(ast::MK::SigVoid(klassLoc, ast::MK::Hash(klassLoc, std::move(sigKeys), std::move(sigVals))));
    result.emplace_back(ast::MK::SyntheticMethod(klassLoc, core::Loc(ctx.file, klassLoc), core::Names::initialize(),
                                                 std::move(args), std::move(body)));
    return result;
}

} // namespace

void Prop::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.runningUnderAutogen) {
        return;
    }
    auto syntacticSuperClass = SyntacticSuperClass::Unknown;
    if (!klass->ancestors.empty()) {
        auto &superClass = klass->ancestors[0];
        if (isTStruct(superClass)) {
            syntacticSuperClass = SyntacticSuperClass::TStruct;
        } else if (isTInexactStruct(superClass)) {
            syntacticSuperClass = SyntacticSuperClass::TInexactStruct;
        } else if (isChalkODMDocument(superClass)) {
            syntacticSuperClass = SyntacticSuperClass::ChalkODMDocument;
        }
    }
    auto propContext = PropContext{syntacticSuperClass, klass->kind};
    UnorderedMap<void *, vector<ast::TreePtr>> replaceNodes;
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
        auto processed = processProp(ctx, propInfo.value(), propContext);
        ENFORCE(!processed.empty(), "if parseProp completed successfully, processProp must complete too");

        vector<ast::TreePtr> nodes;
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
        for (auto &stat : mkTypedInitialize(ctx, klass->loc, props)) {
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
