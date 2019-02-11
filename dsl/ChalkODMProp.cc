#include "dsl/ChalkODMProp.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "dsl/helpers.h"
#include "dsl/util.h"

using namespace std;

namespace sorbet::dsl {

vector<unique_ptr<ast::Expression>> ChalkODMProp::replaceDSL(core::MutableContext ctx, ast::Send *send) {
    bool isOptional = false;  // Can the setter be passed nil?
    bool isImmutable = false; // Are there no setters?
    bool isNilable = true;    // Can the getter return nil?
    vector<unique_ptr<ast::Expression>> empty;
    unique_ptr<ast::Expression> type;
    unique_ptr<ast::Expression> foreign;
    core::NameRef name = core::NameRef::noName();

    switch (send->fun._id) {
        case core::Names::prop()._id:
            // Nothing special
            break;
        case core::Names::optional()._id:
            isOptional = true;
            break;
        case core::Names::const_()._id:
            isImmutable = true;
            break;
        case core::Names::token_prop()._id:
        case core::Names::timestamped_token_prop()._id:
            isNilable = false;
            name = core::Names::token();
            type = ast::MK::Constant(send->loc, core::Symbols::String());
            break;
        case core::Names::created_prop()._id:
            isNilable = false;
            name = core::Names::created();
            type = ast::MK::Constant(send->loc, core::Symbols::Float());
            break;
        case core::Names::merchant_prop()._id:
            isNilable = false;
            isImmutable = true;
            name = core::Names::merchant();
            type = ast::MK::Constant(send->loc, core::Symbols::String());
            break;

        default:
            return empty;
    }

    if ((!name.exists() && send->args.empty()) || send->args.size() > 3) {
        return empty;
    }
    auto loc = send->loc;

    if (!name.exists()) {
        auto *sym = ast::cast_tree<ast::Literal>(send->args[0].get());
        if (!sym || !sym->isSymbol(ctx)) {
            return empty;
        }
        name = sym->asSymbol(ctx);
    }

    if (type == nullptr) {
        if (send->args.size() == 1) {
            type = ast::MK::Constant(send->loc, core::Symbols::Object());
        } else {
            type = ASTUtil::dupType(send->args[1].get());
        }
    }

    ast::Hash *rules = nullptr;
    if (!send->args.empty()) {
        rules = ast::cast_tree<ast::Hash>(send->args.back().get());
    }
    if (rules == nullptr) {
        if (type == nullptr) {
            // No type, and rules isn't a hash: This isn't a T::Props prop
            return empty;
        }
        if (send->args.size() == 3) {
            // Three args. We need name, type, and either rules, or, for
            // DataInterface, a foreign type, wrapped in a thunk.
            if (auto thunk = thunkBody(ctx, send->args.back().get())) {
                foreign = std::move(thunk);
            } else {
                return empty;
            }
        }
    }

    if (type == nullptr) {
        type = ASTUtil::getHashValue(ctx, rules, core::Names::type());
    }

    if (type == nullptr) {
        if (ASTUtil::hasHashValue(ctx, rules, core::Names::enum_())) {
            // Handle enum: by setting the type to untyped, so that we'll parse
            // the declaration. Don't allow assigning it from typed code by deleting setter
            type = ast::MK::Send0(loc, ast::MK::T(loc), core::Names::untyped());
            isNilable = false;
            isImmutable = true;
        }
    }

    if (type == nullptr) {
        auto arrayType = ASTUtil::getHashValue(ctx, rules, core::Names::array());
        if (arrayType) {
            type = ast::MK::Send1(loc, ast::MK::Constant(send->loc, core::Symbols::T_Array()),
                                  core::Names::squareBrackets(), std::move(arrayType));
        }
    }

    if (type == nullptr) {
        return empty;
    }
    if (auto *snd = ast::cast_tree<ast::Send>(type.get())) {
        if (snd->fun == core::Names::coerce()) {
            // TODO: either support T.coerce or remove it from pay-server
            return empty;
        }
    }
    // Yay, we have a type
    ENFORCE(type != nullptr, "No obvious type AST for this prop");

    if (auto send = ast::cast_tree<ast::Send>(type.get())) {
        // A heuristic for detecting the API Param Spec
        if (isOptional && send->fun != core::Names::squareBrackets()) {
            auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv.get());
            if (cnst->cnst != core::Symbols::T().data(ctx)->name) {
                return empty;
            }
        }
    }

    vector<unique_ptr<ast::Expression>> stats;

    // Compute the getters

    if (rules) {
        auto optional = ASTUtil::getHashValue(ctx, rules, core::Names::optional());
        auto optionalLit = ast::cast_tree<ast::Literal>(optional.get());
        if (optionalLit) {
            if (optionalLit->isTrue(ctx)) {
                isOptional = true;
            } else if (optionalLit->isFalse(ctx)) {
                isNilable = false;
            }
        }

        if (ASTUtil::hasHashValue(ctx, rules, core::Names::immutable())) {
            isImmutable = true;
        }

        if (foreign == nullptr) {
            foreign = ASTUtil::getHashValue(ctx, rules, core::Names::foreign());
            if (auto body = thunkBody(ctx, foreign.get())) {
                foreign = std::move(body);
            }
        }

        // In Chalk::ODM `optional String, optional: false` IS optional so
        // we don't falsify any booleans here
    }

    auto getType = ASTUtil::dupType(type.get());
    if (isNilable) {
        getType = mkNilable(loc, std::move(getType));
    }
    stats.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), ASTUtil::dupType(getType.get())));
    stats.emplace_back(mkGet(loc, name, ast::MK::Cast(loc, std::move(getType))));

    // Compute the setter
    if (!isImmutable) {
        auto setType = ASTUtil::dupType(type.get());
        if (isOptional) {
            setType = mkNilable(loc, std::move(setType));
        }

        stats.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(loc, core::Names::arg0()), ASTUtil::dupType(setType.get())),
            ASTUtil::dupType(setType.get())));
        core::NameRef setName = name.addEq(ctx);
        stats.emplace_back(mkSet(loc, setName, ast::MK::Cast(loc, std::move(setType))));
    }

    // Compute the `_` foreign accessor
    if (foreign) {
        unique_ptr<ast::Expression> type;
        if (ASTUtil::dupType(foreign.get()) == nullptr) {
            // If it's not a valid type, just use untyped
            type = ast::MK::Untyped(loc);
        } else {
            type = ast::MK::Nilable(loc, std::move(foreign));
        }
        auto fk_method = ctx.state.enterNameUTF8(name.data(ctx)->toString(ctx) + "_");
        // sig {params(opts: T.untyped).returns(T.nilable($foreign))}
        // def $fk_method(**opts)
        //  T.unsafe(nil)
        // end
        stats.emplace_back(
            ast::MK::Sig1(loc, ast::MK::Symbol(loc, core::Names::opts()), ast::MK::Untyped(loc), std::move(type)));

        unique_ptr<ast::Expression> arg =
            ast::MK::RestArg(loc, ast::MK::KeywordArg(loc, ast::MK::Local(loc, core::Names::opts())));
        stats.emplace_back(ast::MK::Method1(loc, loc, fk_method, std::move(arg),
                                            ast::MK::Unsafe(loc, ast::MK::Nil(loc)), ast::MethodDef::DSLSynthesized));
    }

    // Compute the Mutator
    {
        // Compute a setter
        auto setType = ASTUtil::dupType(type.get());
        if (isOptional) {
            setType = mkNilable(loc, std::move(setType));
        }
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(loc, core::Names::arg0()), ASTUtil::dupType(setType.get())),
            ASTUtil::dupType(setType.get())));
        core::NameRef setName = name.addEq(ctx);
        rhs.emplace_back(mkSet(loc, setName, ast::MK::Cast(loc, std::move(setType))));

        // Maybe make a getter
        unique_ptr<ast::Expression> mutator;
        if (isProbablySymbol(ctx, type.get(), core::Symbols::Hash())) {
            mutator = mkMutator(ctx, loc, core::Names::HashMutator());
            auto send = ast::cast_tree<ast::Send>(type.get());
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 2) {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0].get()), ASTUtil::dupType(send->args[1].get()));
            } else {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc),
                                         ast::MK::Untyped(loc));
            }
        } else if (isProbablySymbol(ctx, type.get(), core::Symbols::Array())) {
            mutator = mkMutator(ctx, loc, core::Names::ArrayMutator());
            auto send = ast::cast_tree<ast::Send>(type.get());
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 1) {
                mutator = ast::MK::Send1(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0].get()));
            } else {
                mutator = ast::MK::Send1(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc));
            }
        } else if (ast::isa_tree<ast::UnresolvedConstantLit>(type.get())) {
            // In a perfect world we could know if there was a Mutator we could reference instead, like this:
            // mutator = ast::MK::UnresolvedConstant(loc, ASTUtil::dupType(type.get()), core::Names::Mutator());
            // For now we're just going to leave these in method_missing.rbi
        }

        if (mutator.get()) {
            rhs.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(mutator.get())));
            rhs.emplace_back(mkGet(loc, name, ast::MK::Cast(loc, std::move(mutator))));

            ast::ClassDef::ANCESTORS_store ancestors;
            auto name = ctx.state.enterNameConstant(core::Names::Mutator());
            stats.emplace_back(ast::MK::Class(loc, loc, ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), name),
                                              std::move(ancestors), std::move(rhs), ast::ClassDefKind::Class));
        }
    }

    return stats;
}

}; // namespace sorbet::dsl
