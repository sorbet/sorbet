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
    vector<unique_ptr<ast::Expression>> empty;

    if (ctx.state.runningUnderAutogen) {
        // TODO(jez) Verify whether this DSL pass is safe to run in for autogen
        return empty;
    }

    bool isImmutable = false; // Are there no setters?
    unique_ptr<ast::Expression> type;
    unique_ptr<ast::Expression> foreign;
    core::NameRef name = core::NameRef::noName();
    core::Loc nameLoc;

    core::NameRef computedByMethodName = core::NameRef::noName();
    core::Loc computedByMethodNameLoc;

    switch (send->fun._id) {
        case core::Names::prop()._id:
            // Nothing special
            break;
        case core::Names::const_()._id:
            isImmutable = true;
            break;
        case core::Names::token_prop()._id:
        case core::Names::timestamped_token_prop()._id:
            name = core::Names::token();
            nameLoc =
                core::Loc(send->loc.file(),
                          send->loc.beginPos() + (send->fun._id == core::Names::timestamped_token_prop()._id ? 12 : 0),
                          send->loc.endPos() - 5); // get the 'token' part of it
            type = ast::MK::Constant(send->loc, core::Symbols::String());
            break;
        case core::Names::created_prop()._id:
            name = core::Names::created();
            nameLoc = core::Loc(send->loc.file(), send->loc.beginPos(),
                                send->loc.endPos() - 5); // 5 is the difference between `created_prop` and `created`
            type = ast::MK::Constant(send->loc, core::Symbols::Float());
            break;
        case core::Names::merchant_prop()._id:
            isImmutable = true;
            name = core::Names::merchant();
            nameLoc = core::Loc(send->loc.file(), send->loc.beginPos(),
                                send->loc.endPos() - 5); // 5 is the difference between `merchant_prop` and `merchant`
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
        ENFORCE(sym->loc.source(ctx).size() > 0 && sym->loc.source(ctx)[0] == ':');
        nameLoc = core::Loc(sym->loc.file(), sym->loc.beginPos() + 1, sym->loc.endPos());
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
        auto [key, value] = ASTUtil::extractHashValue(ctx, *rules, core::Names::type());
        if (value.get() && !ASTUtil::dupType(value.get())) {
            ASTUtil::putBackHashValue(ctx, *rules, move(key), move(value));
        } else {
            type = move(value);
        }
    }

    if (type == nullptr) {
        if (ASTUtil::hasHashValue(ctx, *rules, core::Names::enum_())) {
            // Handle enum: by setting the type to untyped, so that we'll parse
            // the declaration. Don't allow assigning it from typed code by deleting setter
            type = ast::MK::Send0(loc, ast::MK::T(loc), core::Names::untyped());
            isImmutable = true;
        }
    }

    if (type == nullptr) {
        auto [arrayLit, arrayType] = ASTUtil::extractHashValue(ctx, *rules, core::Names::array());
        if (!arrayType.get()) {
            return empty;
        }
        if (!ASTUtil::dupType(arrayType.get())) {
            ASTUtil::putBackHashValue(ctx, *rules, move(arrayLit), move(arrayType));
            return empty;
        } else {
            type = ast::MK::Send1(loc, ast::MK::Constant(send->loc, core::Symbols::T_Array()),
                                  core::Names::squareBrackets(), std::move(arrayType));
        }
    }

    if (auto *snd = ast::cast_tree<ast::Send>(type.get())) {
        if (snd->fun == core::Names::coerce()) {
            // TODO: either support T.coerce or remove it from pay-server
            return empty;
        }
    }
    ENFORCE(type != nullptr, "No obvious type AST for this prop");
    auto getType = ASTUtil::dupType(type.get());
    ENFORCE(getType != nullptr);

    // From this point, we can't `return empty` anymore since we're going to be
    // consuming the tree.

    vector<unique_ptr<ast::Expression>> stats;

    // Compute the getters
    if (rules) {
        if (ASTUtil::hasHashValue(ctx, *rules, core::Names::immutable())) {
            isImmutable = true;
        }

        // e.g. `const :foo, type, computed_by: :method_name`
        if (ASTUtil::hasHashValue(ctx, *rules, core::Names::computedBy())) {
            auto [key, val] = ASTUtil::extractHashValue(ctx, *rules, core::Names::computedBy());
            if (auto *lit = ast::cast_tree<ast::Literal>(val.get())) {
                if (lit->isSymbol(ctx)) {
                    computedByMethodNameLoc = lit->loc;
                    computedByMethodName = lit->asSymbol(ctx);
                } else {
                    // error that value is not a symbol
                    auto typeSymbol =
                        ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), core::Names::Constants::Symbol());
                    stats.emplace_back(ast::MK::Let(lit->loc, move(val), move(typeSymbol)));
                }
            }
        }

        if (foreign == nullptr) {
            auto [fk, foreignTree] = ASTUtil::extractHashValue(ctx, *rules, core::Names::foreign());
            foreign = move(foreignTree);
            if (auto body = thunkBody(ctx, foreign.get())) {
                foreign = std::move(body);
            }
        }
    }

    stats.emplace_back(ast::MK::Sig(loc, ast::MK::Hash0(loc), ASTUtil::dupType(getType.get())));

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
        stats.emplace_back(mkGet(loc, name, std::move(assertTypeMatches)));
    } else {
        stats.emplace_back(mkGet(loc, name, ast::MK::Cast(loc, std::move(getType))));
    }

    core::NameRef setName = name.addEq(ctx);

    // Compute the setter
    if (!isImmutable) {
        auto setType = ASTUtil::dupType(type.get());
        stats.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), ASTUtil::dupType(setType.get())),
            ASTUtil::dupType(setType.get())));
        stats.emplace_back(mkSet(loc, setName, nameLoc, ast::MK::Cast(loc, std::move(setType))));
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
        stats.emplace_back(
            ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::opts()), ast::MK::Untyped(loc), std::move(type)));

        // def $fk_method(**opts)
        //  T.unsafe(nil)
        // end

        auto fk_method = ctx.state.enterNameUTF8(name.data(ctx)->show(ctx) + "_");

        unique_ptr<ast::Expression> arg =
            ast::MK::RestArg(nameLoc, ast::MK::KeywordArg(nameLoc, ast::MK::Local(nameLoc, core::Names::opts())));
        stats.emplace_back(ast::MK::Method1(loc, loc, fk_method, std::move(arg),
                                            ast::MK::Unsafe(loc, ast::MK::Nil(loc)), ast::MethodDef::DSLSynthesized));

        // sig {params(opts: T.untyped).returns($foreign)}
        stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, core::Names::opts()), ast::MK::Untyped(loc),
                                         std::move(nonNilType)));

        // def $fk_method_bang(**opts)
        //  T.unsafe(nil)
        // end

        auto fk_method_bang = ctx.state.enterNameUTF8(name.data(ctx)->show(ctx) + "_!");
        unique_ptr<ast::Expression> arg2 =
            ast::MK::RestArg(nameLoc, ast::MK::KeywordArg(nameLoc, ast::MK::Local(nameLoc, core::Names::opts())));
        stats.emplace_back(ast::MK::Method1(loc, loc, fk_method_bang, std::move(arg2),
                                            ast::MK::Unsafe(loc, ast::MK::Nil(loc)), ast::MethodDef::DSLSynthesized));
    }

    // Compute the Mutator
    {
        // Compute a setter
        auto setType = ASTUtil::dupType(type.get());
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(ast::MK::Sig(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(nameLoc, core::Names::arg0()), ASTUtil::dupType(setType.get())),
            ASTUtil::dupType(setType.get())));
        rhs.emplace_back(mkSet(loc, setName, nameLoc, ast::MK::Cast(loc, std::move(setType))));

        // Maybe make a getter
        unique_ptr<ast::Expression> mutator;
        if (isProbablySymbol(ctx, type.get(), core::Symbols::Hash())) {
            mutator = mkMutator(ctx, loc, core::Names::Constants::HashMutator());
            auto send = ast::cast_tree<ast::Send>(type.get());
            if (send && send->fun == core::Names::squareBrackets() && send->args.size() == 2) {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(),
                                         ASTUtil::dupType(send->args[0].get()), ASTUtil::dupType(send->args[1].get()));
            } else {
                mutator = ast::MK::Send2(loc, std::move(mutator), core::Names::squareBrackets(), ast::MK::Untyped(loc),
                                         ast::MK::Untyped(loc));
            }
        } else if (isProbablySymbol(ctx, type.get(), core::Symbols::Array())) {
            mutator = mkMutator(ctx, loc, core::Names::Constants::ArrayMutator());
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
            rhs.emplace_back(mkGet(loc, name, ast::MK::Cast(loc, std::move(mutator))));

            ast::ClassDef::ANCESTORS_store ancestors;
            auto name = core::Names::Constants::Mutator();
            stats.emplace_back(ast::MK::Class(loc, loc, ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), name),
                                              std::move(ancestors), std::move(rhs), ast::ClassDefKind::Class));
        }
    }

    return stats;
}

}; // namespace sorbet::dsl
