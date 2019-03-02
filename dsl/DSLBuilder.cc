#include "dsl/DSLBuilder.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
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

    if (ctx.state.forAutogen) {
        // TODO(jez) Verify whether this DSL pass is safe to run in for autogen
        return empty;
    }

    bool nilable = false;
    bool implied = false;
    bool skipGetter = false;
    bool skipSetter = false;
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

    ENFORCE(!sym->loc.source(ctx).empty() && sym->loc.source(ctx)[0] == ':');
    auto nameLoc = core::Loc(sym->loc.file(), sym->loc.beginPos() + 1, sym->loc.endPos());

    type = ASTUtil::dupType(send->args[1].get());
    if (!type) {
        return empty;
    }

    ast::Hash *opts = nullptr;
    if (send->args.size() > 2) {
        opts = ast::cast_tree<ast::Hash>(send->args[2].get());
    }
    if (opts != nullptr) {
        if (ASTUtil::hasHashValue(ctx, *opts, core::Names::default_())) {
            nilable = false;
        }
        if (ASTUtil::hasHashValue(ctx, *opts, core::Names::implied())) {
            implied = true;
        }
        if (ASTUtil::hasHashValue(ctx, *opts, core::Names::skipGetter())) {
            skipGetter = true;
        }
        if (ASTUtil::hasHashValue(ctx, *opts, core::Names::skipSetter())) {
            skipSetter = true;
        }
    }

    auto loc = send->loc;

    vector<unique_ptr<ast::Expression>> stats;

    // def self.<prop>
    if (!skipSetter) {
        stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, name), ASTUtil::dupType(type.get()),
                                         ast::MK::Constant(loc, core::Symbols::NilClass())));
        unique_ptr<ast::Reference> arg = ast::MK::Local(nameLoc, name);
        if (implied) {
            auto default_ = ast::MK::Send0(loc, ast::MK::T(loc), core::Names::untyped());
            arg = make_unique<ast::OptionalArg>(loc, move(arg), move(default_));
        }
        stats.emplace_back(ast::MK::Method1(loc, loc, name, move(arg), ast::MK::EmptyTree(),
                                            ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized));
    }

    if (!skipGetter) {
        if (nilable) {
            auto tyloc = type->loc;
            type = ast::MK::Nilable(tyloc, move(type));
        }
        // def self.get_<prop>
        core::NameRef getName = ctx.state.enterNameUTF8("get_" + name.data(ctx)->show(ctx));
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
