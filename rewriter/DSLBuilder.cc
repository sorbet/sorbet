#include "rewriter/DSLBuilder.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/Util.h"
#include "rewriter/rewriter.h"

using namespace std;

//
// This file models the behavior of chalk-framework-builder's dsl_builder.rb:
//
//     http://go/git/stripe-internal/chalk-framework-builder/blob/master/lib/chalk-framework-builder/dsl_builder.rb
//
// You might want to cross-reference that file when trying to understand or update this code.
//

namespace sorbet::rewriter {
vector<ast::TreePtr> DSLBuilder::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::TreePtr> empty;

    if (ctx.state.runningUnderAutogen) {
        return empty;
    }

    bool nilable = false;
    bool implied = false;
    bool skipGetter = false;
    bool skipSetter = false;
    ast::TreePtr type;
    core::NameRef name;

    if (send->fun == core::Names::dslOptional()) {
        nilable = true;
    } else if (send->fun == core::Names::dslRequired()) {
    } else {
        return empty;
    }

    if (send->args.size() < 2) {
        return empty;
    }
    auto *sym = ast::cast_tree<ast::Literal>(send->args[0]);
    if (sym == nullptr || !sym->isSymbol(ctx)) {
        return empty;
    }
    name = sym->asSymbol(ctx);

    ENFORCE(!core::Loc(ctx.file, sym->loc).source(ctx).empty() && core::Loc(ctx.file, sym->loc).source(ctx)[0] == ':');
    auto nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};

    type = ASTUtil::dupType(send->args[1]);
    if (!type) {
        return empty;
    }

    ast::TreePtr optsTree = ASTUtil::mkKwArgsHash(send);
    if (auto *opts = ast::cast_tree<ast::Hash>(optsTree)) {
        if (ASTUtil::hasHashValue(ctx, *opts, core::Names::default_())) {
            nilable = false;
        }
        if (ASTUtil::hasTruthyHashValue(ctx, *opts, core::Names::implied())) {
            implied = true;
        }
        if (ASTUtil::hasTruthyHashValue(ctx, *opts, core::Names::skipGetter())) {
            skipGetter = true;
        }
        if (ASTUtil::hasTruthyHashValue(ctx, *opts, core::Names::skipSetter())) {
            skipSetter = true;
        }
    }

    auto loc = send->loc;

    vector<ast::TreePtr> stats;

    // def self.<prop>
    if (!skipSetter) {
        stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, name), ASTUtil::dupType(type),
                                         ast::MK::Constant(loc, core::Symbols::NilClass())));
        auto arg = ast::MK::Local(nameLoc, name);
        if (implied) {
            auto default_ = ast::MK::Send0(loc, ast::MK::T(loc), core::Names::untyped());
            arg = ast::MK::OptionalArg(loc, move(arg), move(default_));
        }
        auto defSelfProp =
            ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), name, move(arg), ast::MK::EmptyTree());
        ast::cast_tree<ast::MethodDef>(defSelfProp)->flags.isSelfMethod = true;
        stats.emplace_back(move(defSelfProp));
    }

    if (!skipGetter) {
        if (nilable) {
            auto tyloc = type->loc;
            type = ast::MK::Nilable(tyloc, move(type));
        }
        // def self.get_<prop>
        core::NameRef getName = ctx.state.enterNameUTF8("get_" + name.data(ctx)->show(ctx));
        stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type)));
        auto defSelfGetProp =
            ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), getName, {}, ast::MK::RaiseUnimplemented(loc));
        ast::cast_tree<ast::MethodDef>(defSelfGetProp)->flags.isSelfMethod = true;
        stats.emplace_back(move(defSelfGetProp));

        // def <prop>()
        stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type)));
        stats.emplace_back(
            ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), name, {}, ast::MK::RaiseUnimplemented(loc)));
    }

    return stats;
}
} // namespace sorbet::rewriter
