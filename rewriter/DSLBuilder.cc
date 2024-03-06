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
vector<ast::ExpressionPtr> DSLBuilder::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.runningUnderAutogen) {
        return empty;
    }

    bool nilable = false;
    bool implied = false;
    bool skipGetter = false;
    bool skipSetter = false;
    ast::ExpressionPtr type;
    core::NameRef name;
    core::NameRef sendFun = send->fun;

    if (sendFun == core::Names::dslOptional()) {
        nilable = true;
    } else if (sendFun == core::Names::dslRequired()) {
    } else {
        return empty;
    }

    if (send->numPosArgs() < 2) {
        return empty;
    }
    auto *sym = ast::cast_tree<ast::Literal>(send->getPosArg(0));
    if (sym == nullptr || !sym->isSymbol()) {
        return empty;
    }
    name = sym->asSymbol();

    ENFORCE(ctx.locAt(sym->loc).exists());
    ENFORCE(!ctx.locAt(sym->loc).source(ctx).value().empty() && ctx.locAt(sym->loc).source(ctx).value()[0] == ':');
    auto nameLoc = core::LocOffsets{sym->loc.beginPos() + 1, sym->loc.endPos()};

    type = ASTUtil::dupType(send->getPosArg(1));
    if (!type) {
        return empty;
    }

    ast::ExpressionPtr optsTree = ASTUtil::mkKwArgsHash(send);
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

    vector<ast::ExpressionPtr> stats;

    ast::MethodDef::Flags flags;
    flags.discardDef = true;

    // We need to keep around the original send for the compiler.
    stats.emplace_back(send->withNewBody(loc, ast::MK::Unsafe(loc, move(send->recv)), sendFun));

    // def self.<prop>
    if (!skipSetter) {
        stats.emplace_back(ast::MK::Sig1(loc, ast::MK::Symbol(nameLoc, name), ASTUtil::dupType(type),
                                         ast::MK::Constant(loc, core::Symbols::NilClass())));
        auto arg = ast::MK::ResolvedLocal(nameLoc, name);
        if (implied) {
            auto default_ = ast::MK::UntypedNil(loc);
            arg = ast::MK::OptionalArg(loc, move(arg), move(default_));
        }
        auto defSelfProp = ast::MK::SyntheticMethod1(loc, loc, name, move(arg), ast::MK::EmptyTree(), flags);
        ast::cast_tree<ast::MethodDef>(defSelfProp)->flags.isSelfMethod = true;
        stats.emplace_back(move(defSelfProp));
    }

    if (!skipGetter) {
        if (nilable) {
            auto tyloc = type.loc();
            type = ast::MK::Nilable(tyloc, move(type));
        }
        // def self.get_<prop>
        core::NameRef getName = ctx.state.enterNameUTF8("get_" + name.show(ctx));
        stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type)));
        auto defSelfGetProp = ast::MK::SyntheticMethod(loc, loc, getName, {}, ast::MK::RaiseUnimplemented(loc), flags);
        ast::cast_tree<ast::MethodDef>(defSelfGetProp)->flags.isSelfMethod = true;
        stats.emplace_back(move(defSelfGetProp));

        // def <prop>()
        stats.emplace_back(ast::MK::Sig0(loc, ASTUtil::dupType(type)));
        stats.emplace_back(ast::MK::SyntheticMethod(loc, loc, name, {}, ast::MK::RaiseUnimplemented(loc), flags));
    }

    return stats;
}
} // namespace sorbet::rewriter
