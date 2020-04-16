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
bool isT(ast::Expression *expr) {
    auto *t = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    if (t == nullptr || t->cnst != core::Names::Constants::T()) {
        return false;
    }
    auto scope = t->scope.get();
    if (ast::isa_tree<ast::EmptyTree>(scope)) {
        return true;
    }
    auto root = ast::cast_tree<ast::ConstantLit>(scope);
    return root != nullptr && root->symbol == core::Symbols::root();
}

bool isTNilable(ast::Expression *expr) {
    auto *nilable = ast::cast_tree<ast::Send>(expr);
    return nilable != nullptr && nilable->fun == core::Names::nilable() && isT(nilable->recv.get());
}

bool isTStruct(ast::Expression *expr) {
    auto *struct_ = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
    return struct_ != nullptr && struct_->cnst == core::Names::Constants::Struct() && isT(struct_->scope.get());
}

struct PropInfo {
    core::LocOffsets loc;
    bool isImmutable = false;
    core::NameRef name;
    core::LocOffsets nameLoc;
    unique_ptr<ast::Expression> type;
    optional<unique_ptr<ast::Expression>> default_;
    core::NameRef computedByMethodName;
    core::LocOffsets computedByMethodNameLoc;
    unique_ptr<ast::Expression> foreign;
};

struct NodesAndPropInfo {
    vector<unique_ptr<ast::Expression>> nodes;
    PropInfo propInfo;
};

optional<PropInfo> parseProp(core::MutableContext ctx, const ast::Send *send) {
    PropInfo ret;
    ret.loc = send->loc;
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

    if (!ret.name.exists()) {
        if (send->args.empty()) {
            return nullopt;
        }
        auto *sym = ast::cast_tree<ast::Literal>(send->args[0].get());
        if (!sym || !sym->isSymbol(ctx)) {
            return nullopt;
        }
        ret.name = sym->asSymbol(ctx);
        ENFORCE(!core::Loc(ctx.file, sym->loc).source(ctx).empty() &&
                core::Loc(ctx.file, sym->loc).source(ctx)[0] == ':');
        ret.nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};
    }

    if (ret.type == nullptr) {
        if (send->args.size() == 1) {
            // Type must have been inferred from prop method (like created_prop) or
            // been given in second argument (either directly, or indirectly via rules)
            return nullopt;
        } else {
            ret.type = ASTUtil::dupType(send->args[1].get());
        }
    }

    unique_ptr<ast::Hash> rules;
    if (!send->args.empty()) {
        if (auto back = ast::cast_tree<ast::Hash>(send->args.back().get())) {
            // Deep copy the rules hash so that we can destruct it at will to parse things,
            // without having to worry about whether we stole things from the tree.
            rules.reset(ast::cast_tree<ast::Hash>(back->deepCopy().release()));
        }
    }

    if (rules == nullptr) {
        if (ret.type == nullptr) {
            // No type, and rules isn't a hash: This isn't a T::Props prop
            return std::nullopt;
        }
        if (send->args.size() == 3) {
            // No rules, but 3 args including name and type. Also not a T::Props
            return std::nullopt;
        }
    }

    if (ret.type == nullptr) {
        auto [key, value] = ASTUtil::extractHashValue(ctx, *rules, core::Names::type());
        if (value != nullptr) {
            // dupType also checks if value is a valid type
            if (auto rulesType = ASTUtil::dupType(value.get())) {
                ret.type = move(rulesType);
            }
        }
    }

    if (ret.type == nullptr) {
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::enum_())) {
            // Handle enum: by setting the type to untyped, so that we'll parse
            // the declaration. Don't allow assigning it from typed code by deleting setter
            ret.type = ast::MK::Send0(ret.loc, ast::MK::T(ret.loc), core::Names::untyped());
            ret.isImmutable = true;
        }
    }

    if (ret.type == nullptr) {
        auto [arrayLit, arrayType] = ASTUtil::extractHashValue(ctx, *rules, core::Names::array());
        if (!arrayType.get()) {
            return std::nullopt;
        }
        if (!ASTUtil::dupType(arrayType.get())) {
            return std::nullopt;
        } else {
            ret.type = ast::MK::Send1(ret.loc, ast::MK::Constant(send->loc, core::Symbols::T_Array()),
                                      core::Names::squareBrackets(), std::move(arrayType));
        }
    }

    if (auto *snd = ast::cast_tree<ast::Send>(ret.type.get())) {
        if (snd->fun == core::Names::coerce()) {
            // TODO: either support T.coerce or remove it from pay-server
            return std::nullopt;
        }
    }

    ENFORCE(ASTUtil::dupType(ret.type.get()) != nullptr, "No obvious type AST for this prop");

    if (isTNilable(ret.type.get())) {
        ret.default_ = ast::MK::Nil(ret.loc);
    } else if (rules == nullptr) {
        ret.default_ = std::nullopt;
    } else if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::factory())) {
        ret.default_ = ast::MK::RaiseUnimplemented(ret.loc);
    } else if (ASTUtil::hasHashValue(ctx, *rules, core::Names::default_())) {
        auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::default_());
        ret.default_ = std::move(val);
    } else {
        ret.default_ = std::nullopt;
    }

    if (rules) {
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::immutable())) {
            ret.isImmutable = true;
        }

        // e.g. `const :foo, type, computed_by: :method_name`
        if (ASTUtil::hasTruthyHashValue(ctx, *rules, core::Names::computedBy())) {
            auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::computedBy());
            auto lit = ast::cast_tree<ast::Literal>(val.get());
            if (lit != nullptr && lit->isSymbol(ctx)) {
                ret.computedByMethodNameLoc = lit->loc;
                ret.computedByMethodName = lit->asSymbol(ctx);
            } else {
                if (auto e = ctx.beginError(val->loc, core::errors::Rewriter::ComputedBySymbol)) {
                    e.setHeader("Value for `{}` must be a symbol literal", "computed_by");
                }
            }
        }

        if (ret.foreign == nullptr) {
            auto [fk, foreignTree] = ASTUtil::extractHashValue(ctx, *rules, core::Names::foreign());
            if (foreignTree != nullptr) {
                ret.foreign = move(foreignTree);
                if (auto body = ASTUtil::thunkBody(ctx, ret.foreign.get())) {
                    ret.foreign = std::move(body);
                } else {
                    if (auto e = ctx.beginError(ret.foreign->loc, core::errors::Rewriter::PropForeignStrict)) {
                        e.setHeader("The argument to `{}` must be a lambda", "foreign:");
                        e.replaceWith("Convert to lambda", core::Loc(ctx.file, ret.foreign->loc), "-> {{{}}}",
                                      core::Loc(ctx.file, ret.foreign->loc).source(ctx));
                    }
                }
            }
        }
    }

    return ret;
}

optional<NodesAndPropInfo> processProp(core::MutableContext ctx) {
    ret.nodes.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), ASTUtil::dupType(getType.get())));

    if (computedByMethodName.exists()) {
        // Given `const :foo, type, computed_by: <name>`, where <name> is a Symbol pointing to a class method,
        // assert that the method takes 1 argument (of any type), and returns the same type as the prop,
        // via `T.assert_type!(self.class.compute_foo(T.unsafe(nil)), type)` in the getter.
        auto selfSendClass = ast::MK::Send0(computedByMethodNameLoc, ast::MK::Self(loc), core::Names::class_());
        auto unsafeNil = ast::MK::Unsafe(computedByMethodNameLoc, ast::MK::Nil(computedByMethodNameLoc));
        auto sendComputedMethod = ast::MK::Send1(computedByMethodNameLoc, std::move(selfSendClass),
                                                 computedByMethodName, std::move(unsafeNil));
        auto assertTypeMatches = ast::MK::AssertType(computedByMethodNameLoc, std::move(sendComputedMethod),
                                                     ASTUtil::dupType(getType.get()));
        auto insSeq = ast::MK::InsSeq1(loc, std::move(assertTypeMatches), ast::MK::RaiseUnimplemented(loc));
        ret.nodes.emplace_back(ASTUtil::mkGet(core::Loc(ctx.file, loc), name, std::move(insSeq)));
    } else {
        ret.nodes.emplace_back(ASTUtil::mkGet(core::Loc(ctx.file, loc), name, ast::MK::RaiseUnimplemented(loc)));
    }

    core::NameRef setName = name.addEq(ctx);

    // Compute the setter
    if (!isImmutable) {
        auto setType = ASTUtil::dupType(type.get());
        ret.nodes.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), ASTUtil::dupType(setType.get())),
            ASTUtil::dupType(setType.get())));
        ret.nodes.emplace_back(
            ASTUtil::mkSet(core::Loc(ctx.file, loc), setName, nameLoc, ast::MK::RaiseUnimplemented(loc)));
    }

    // Compute the `_` foreign accessor
    if (foreign) {
        unique_ptr<ast::Expression> type;
        unique_ptr<ast::Expression> nonNilType;
        if (ASTUtil::dupType(foreign.get()) == nullptr) {
            // If it's not a valid type, just use untyped
            type = ast::MK::Untyped(loc);
            nonNilType = ast::MK::Untyped(loc);
        } else {
            type = ast::MK::Nilable(loc, ASTUtil::dupType(foreign.get()));
            nonNilType = ASTUtil::dupType(foreign.get());
        }

        // sig {params(opts: T.untyped).returns(T.nilable($foreign))}
        ret.nodes.emplace_back(
            ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::opts()), ast::MK::Untyped(loc), std::move(type)));

        // def $fk_method(**opts)
        //  T.unsafe(nil)
        // end

        auto fkMethod = ctx.state.enterNameUTF8(name.data(ctx)->show(ctx) + "_");

        unique_ptr<ast::Expression> arg =
            ast::MK::RestArg(nameLoc, ast::MK::KeywordArg(nameLoc, ast::MK::Local(nameLoc, core::Names::opts())));
        ret.nodes.emplace_back(ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), fkMethod, std::move(arg),
                                                         ast::MK::RaiseUnimplemented(loc)));

        // sig {params(opts: T.untyped).returns($foreign)}
        ret.nodes.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::opts()), ast::MK::Untyped(loc),
                                             std::move(nonNilType)));

        // def $fk_method_!(**opts)
        //  T.unsafe(nil)
        // end

        auto fkMethodBang = ctx.state.enterNameUTF8(name.data(ctx)->show(ctx) + "_!");
        unique_ptr<ast::Expression> arg2 =
            ast::MK::RestArg(nameLoc, ast::MK::KeywordArg(nameLoc, ast::MK::Local(nameLoc, core::Names::opts())));
        ret.nodes.emplace_back(ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), fkMethodBang, std::move(arg2),
                                                         ast::MK::RaiseUnimplemented(loc)));
    }

    // Compute the Mutator
    {
        // Compute a setter
        auto setType = ASTUtil::dupType(type.get());
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), ASTUtil::dupType(setType.get())),
            ASTUtil::dupType(setType.get())));
        rhs.emplace_back(ASTUtil::mkSet(core::Loc(ctx.file, loc), setName, nameLoc, ast::MK::RaiseUnimplemented(loc)));

        // Maybe make a getter
        unique_ptr<ast::Expression> mutator;
        if (ASTUtil::isProbablySymbol(ctx, type.get(), core::Symbols::Hash())) {
            mutator = ASTUtil::mkMutator(ctx, loc, core::Names::Constants::HashMutator());
            auto send = ast::cast_tree<ast::Send>(type.get());
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 2) {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0].get()), ASTUtil::dupType(send->args[1].get()));
            } else {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc),
                                         ast::MK::Untyped(loc));
            }
        } else if (ASTUtil::isProbablySymbol(ctx, type.get(), core::Symbols::Array())) {
            mutator = ASTUtil::mkMutator(ctx, loc, core::Names::Constants::ArrayMutator());
            auto send = ast::cast_tree<ast::Send>(type.get());
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 1) {
                mutator = ast::MK::Send1(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0].get()));
            } else {
                mutator = ast::MK::Send1(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc));
            }
        } else if (ast::isa_tree<ast::UnresolvedConstantLit>(type.get())) {
            // In a perfect world we could know if there was a Mutator we could reference instead, like this:
            // mutator = ast::MK::UnresolvedConstant(loc, ASTUtil::dupType(type.get()),
            // core::Names::Constants::Mutator()); For now we're just going to leave these in method_missing.rbi
        }

        if (mutator.get()) {
            rhs.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(mutator.get())));
            rhs.emplace_back(ASTUtil::mkGet(core::Loc(ctx.file, loc), name, ast::MK::RaiseUnimplemented(loc)));

            ast::ClassDef::ANCESTORS_store ancestors;
            auto name = core::Names::Constants::Mutator();
            ret.nodes.emplace_back(ast::MK::Class(loc, core::Loc(ctx.file, loc),
                                                  ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), name),
                                                  std::move(ancestors), std::move(rhs)));
        }
    }

    return ret;
}
} // namespace

void Prop::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.runningUnderAutogen) {
        return;
    }
    auto synthesizeInitialize = false;
    for (auto &a : klass->ancestors) {
        if (isTStruct(a.get())) {
            synthesizeInitialize = true;
            break;
        }
    }
    UnorderedMap<ast::Expression *, vector<unique_ptr<ast::Expression>>> replaceNodes;
    vector<PropInfo> props;
    for (auto &stat : klass->rhs) {
        auto *send = ast::cast_tree<ast::Send>(stat.get());
        if (send == nullptr) {
            continue;
        }
        auto nodesAndPropInfo = processProp(ctx, send);
        if (!nodesAndPropInfo.has_value()) {
            continue;
        }
        ENFORCE(!nodesAndPropInfo->nodes.empty(), "nodesAndPropInfo with value must have nodes be non empty");
        replaceNodes[stat.get()] = std::move(nodesAndPropInfo->nodes);
        props.emplace_back(std::move(nodesAndPropInfo->propInfo));
    }
    auto oldRHS = std::move(klass->rhs);
    klass->rhs.clear();
    klass->rhs.reserve(oldRHS.size());
    if (synthesizeInitialize) {
        // we define our synthesized initialize first so that if the user wrote one themselves, it overrides ours.
        ast::MethodDef::ARGS_store args;
        ast::Hash::ENTRY_store sigKeys;
        ast::Hash::ENTRY_store sigVals;
        args.reserve(props.size());
        sigKeys.reserve(props.size());
        sigVals.reserve(props.size());
        // add all the required props first.
        for (auto &prop : props) {
            if (prop.default_.has_value()) {
                continue;
            }
            auto loc = prop.loc;
            args.emplace_back(ast::MK::KeywordArg(loc, ast::MK::Local(loc, prop.name)));
            sigKeys.emplace_back(ast::MK::Symbol(loc, prop.name));
            sigVals.emplace_back(std::move(prop.type));
        }
        // then, add all the optional props.
        for (auto &prop : props) {
            if (!prop.default_.has_value()) {
                continue;
            }
            auto loc = prop.loc;
            args.emplace_back(ast::MK::OptionalArg(loc, ast::MK::KeywordArg(loc, ast::MK::Local(loc, prop.name)),
                                                   std::move(*(prop.default_))));
            sigKeys.emplace_back(ast::MK::Symbol(loc, prop.name));
            sigVals.emplace_back(std::move(prop.type));
        }
        auto loc = klass->loc;
        klass->rhs.emplace_back(ast::MK::SigVoid(loc, ast::MK::Hash(loc, std::move(sigKeys), std::move(sigVals))));
        klass->rhs.emplace_back(ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), core::Names::initialize(),
                                                         std::move(args), ast::MK::EmptyTree()));
    }
    // this is cargo-culted from rewriter.cc.
    for (auto &stat : oldRHS) {
        if (replaceNodes.find(stat.get()) == replaceNodes.end()) {
            klass->rhs.emplace_back(std::move(stat));
        } else {
            for (auto &newNode : replaceNodes.at(stat.get())) {
                klass->rhs.emplace_back(std::move(newNode));
            }
        }
    }
}

}; // namespace sorbet::rewriter
