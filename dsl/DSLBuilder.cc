#include "dsl/DSLBuilder.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "dsl/dsl.h"
#include "dsl/util.h"

using namespace std;

//
// This file models the behavior of chalk-framework-builder's dsl_builder.rb:
//
//     http://go/git/stripe-internal/chalk-framework-builder/blob/master/lib/chalk-framework-builder/dsl_builder.rb
//
// You might want to cross-reference that file when trying to understand or update this code.
//

namespace sorbet::dsl {
vector<unique_ptr<ast::Expression>> DSLBuilder::replaceDSL(core::MutableContext ctx, ast::Send *send) {
    vector<unique_ptr<ast::Expression>> empty;

    bool nilable = false;
    bool implied = false;
    bool skipGetter = false;
    unique_ptr<ast::Expression> type;
    core::NameRef name = core::NameRef::noName();

    if (send->fun == core::Names::dslOptional()) {
        nilable = true;
    } else if (send->fun == core::Names::dslRequired()) {
    } else {
        return empty;
    }

    if (send->args.size() < 2) {
        return empty;
    }
    auto *sym = ast::cast_tree<ast::Literal>(send->args[0].get());
    if (sym == nullptr || !sym->isSymbol(ctx)) {
        return empty;
    }
    name = sym->asSymbol(ctx);

    auto *klass = ast::cast_tree<ast::UnresolvedConstantLit>(send->args[1].get());
    if (klass == nullptr) {
        return empty;
    }
    type = ASTUtil::dupType(send->args[1].get());

    ast::Hash *opts = nullptr;
    if (send->args.size() > 2) {
        opts = ast::cast_tree<ast::Hash>(send->args[2].get());
    }
    if (opts != nullptr) {
        if (ASTUtil::getHashValue(ctx, opts, core::Names::default_())) {
            nilable = false;
        }
        if (ASTUtil::getHashValue(ctx, opts, core::Names::implied())) {
            implied = true;
        }
        if (ASTUtil::getHashValue(ctx, opts, core::Names::skipGetter())) {
            skipGetter = true;
        }
    }

    auto loc = send->loc;

    vector<unique_ptr<ast::Expression>> stats;

    // def self.<prop>
    stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(loc, name), ASTUtil::dupType(type.get()),
                                     ast::MK::Constant(loc, core::Symbols::NilClass())));
    unique_ptr<ast::Reference> arg = ast::MK::Local(loc, name);
    if (implied) {
        auto default_ = ast::MK::Send0(loc, ast::MK::T(loc), core::Names::untyped());
        arg = make_unique<ast::OptionalArg>(loc, move(arg), move(default_));
    }
    stats.emplace_back(ast::MK::Method1(loc, loc, name, move(arg), ast::MK::EmptyTree(loc),
                                        ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized));

    if (!skipGetter) {
        // def self.get_<prop>
        core::NameRef getName = ctx.state.enterNameUTF8("get_" + name.data(ctx).toString(ctx));
        stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type.get())));
        stats.emplace_back(ast::MK::Method(loc, loc, getName, ast::MethodDef::ARGS_store(),
                                           ast::MK::Unsafe(loc, ast::MK::Nil(loc)),
                                           ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized));

        // def <prop>()
        stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type.get())));
        stats.emplace_back(ast::MK::Method(loc, loc, name, ast::MethodDef::ARGS_store(),
                                           ast::MK::Unsafe(loc, ast::MK::Nil(loc)), ast::MethodDef::DSLSynthesized));
    }

    return stats;
}
} // namespace sorbet::dsl
