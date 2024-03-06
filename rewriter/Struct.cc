#include "rewriter/Struct.h"
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool isKeywordInitKey(const core::GlobalState &gs, const ast::ExpressionPtr &node) {
    if (auto *lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isSymbol() && lit->asSymbol() == core::Names::keywordInit();
    }
    return false;
}

// Elem = type_member {{fixed: T.untyped}}
ast::ExpressionPtr elemFixedUntyped(core::LocOffsets loc) {
    auto typeMember = ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::typeMember(), loc.copyWithZeroLength());
    ast::cast_tree_nonnull<ast::Send>(typeMember)
        .setBlock(ast::MK::Block0(
            loc, ast::MK::Hash1(loc, ast::MK::Symbol(loc, core::Names::fixed()), ast::MK::Untyped(loc))));
    return ast::MK::Assign(loc, ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), core::Names::Constants::Elem()),
                           std::move(typeMember));
}

void selfScopeToEmptyTree(ast::UnresolvedConstantLit &cnst) {
    if (ast::isa_tree<ast::EmptyTree>(cnst.scope) || ast::isa_tree<ast::ConstantLit>(cnst.scope)) {
        return;
    }

    if (cnst.scope.isSelfReference()) {
        cnst.scope = ast::MK::EmptyTree();
        return;
    }

    if (auto scope = ast::cast_tree<ast::UnresolvedConstantLit>(cnst.scope)) {
        selfScopeToEmptyTree(*scope);
        return;
    }
}

} // namespace

vector<ast::ExpressionPtr> Struct::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.runningUnderAutogen) {
        return empty;
    }

    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    if (lhs == nullptr) {
        return empty;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs);
    if (send == nullptr) {
        return empty;
    }

    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (recv == nullptr) {
        return empty;
    }

    if (!ast::MK::isRootScope(recv->scope) || recv->cnst != core::Symbols::Struct().data(ctx)->name ||
        send->fun != core::Names::new_() || (!send->hasPosArgs() && !send->hasKwArgs())) {
        return empty;
    }

    auto loc = asgn->loc;

    ast::MethodDef::ARGS_store newArgs;
    ast::Send::ARGS_store sigArgs;
    ast::ClassDef::RHS_store body;

    bool keywordInit = false;
    if (send->hasKwArgs()) {
        if (!send->hasPosArgs()) {
            // leave bad usages like `Struct.new(keyword_init: true)` untouched so we error later
            return empty;
        }
        if (send->hasKwSplat()) {
            return empty;
        }

        if (send->numKwArgs() != 1) {
            return empty;
        }

        if (!isKeywordInitKey(ctx, send->getKwKey(0))) {
            return empty;
        }

        if (auto *lit = ast::cast_tree<ast::Literal>(send->getKwValue(0))) {
            if (lit->isTrue(ctx)) {
                keywordInit = true;
            } else if (!lit->isFalse(ctx)) {
                return empty;
            }
        } else {
            return empty;
        }
    }

    for (int i = 0; i < send->numPosArgs(); i++) {
        auto *sym = ast::cast_tree<ast::Literal>(send->getPosArg(i));
        if (!sym || !sym->isSymbol()) {
            return empty;
        }
        core::NameRef name = sym->asSymbol();
        auto symLoc = sym->loc;
        auto strname = name.shortName(ctx);
        if (!strname.empty() && strname.back() == '=') {
            if (auto e = ctx.beginError(symLoc, core::errors::Rewriter::InvalidStructMember)) {
                e.setHeader("Struct member `{}` cannot end with an equal", strname);
            }
        }

        // TODO(jez) Use Loc::adjust here
        if (symLoc.exists() && absl::StartsWith(ctx.locAt(symLoc).source(ctx).value(), ":")) {
            symLoc = core::LocOffsets{symLoc.beginPos() + 1, symLoc.endPos()};
        }

        sigArgs.emplace_back(ast::MK::Symbol(symLoc, name));
        sigArgs.emplace_back(ast::MK::Constant(symLoc, core::Symbols::BasicObject()));
        auto argName = ast::MK::ResolvedLocal(symLoc, name);
        if (keywordInit) {
            argName = ast::make_expression<ast::KeywordArg>(symLoc, move(argName));
        }
        newArgs.emplace_back(ast::MK::OptionalArg(symLoc, move(argName), ast::MK::Nil(symLoc)));

        body.emplace_back(ast::MK::Sig0(symLoc.copyWithZeroLength(), ast::MK::Untyped(symLoc.copyWithZeroLength())));
        body.emplace_back(ast::MK::SyntheticMethod0(symLoc, symLoc, name, ast::MK::RaiseTypedUnimplemented(loc)));
        body.emplace_back(ast::MK::Sig1(symLoc.copyWithZeroLength(), ast::MK::Symbol(symLoc, name),
                                        ast::MK::Untyped(symLoc.copyWithZeroLength()),
                                        ast::MK::Untyped(symLoc.copyWithZeroLength())));
        body.emplace_back(ast::MK::SyntheticMethod1(symLoc, symLoc, name.addEq(ctx), ast::MK::ResolvedLocal(symLoc, name),
                                                    ast::MK::RaiseTypedUnimplemented(loc)));
    }

    body.emplace_back(elemFixedUntyped(loc));
    body.emplace_back(ast::MK::SigVoid(loc, std::move(sigArgs)));
    body.emplace_back(ast::MK::SyntheticMethod(loc, loc, core::Names::initialize(), std::move(newArgs),
                                               ast::MK::RaiseTypedUnimplemented(loc)));

    vector<ast::ExpressionPtr> stats;

    auto structUniqueName = asgn->lhs.deepCopy();
    auto &structUniqueNameUnresolvedConst = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(structUniqueName);
    structUniqueNameUnresolvedConst.cnst =
        ctx.state.enterNameConstant(ctx.state.freshNameUnique(core::UniqueNameKind::Struct, lhs->cnst, 1));

    // class Foo$1 < ::Struct
    {
        ast::ClassDef::ANCESTORS_store ancestors;
        ancestors.emplace_back(ast::MK::UnresolvedConstant(loc, ast::MK::Constant(loc, core::Symbols::root()),
                                                           core::Names::Constants::Struct()));

        stats.emplace_back(
            ast::MK::Class(loc, loc, structUniqueName.deepCopy(), std::move(ancestors), std::move(body)));
    }

    // class Foo < Foo$1
    {
        ast::ClassDef::ANCESTORS_store ancestors;
        selfScopeToEmptyTree(structUniqueNameUnresolvedConst);
        ancestors.emplace_back(move(structUniqueName));

        ast::ClassDef::RHS_store body;

        body.emplace_back(elemFixedUntyped(loc));

        if (auto *block = send->block()) {
            // Steal the trees, because the run is going to remove the original send node from the tree anyway.
            if (auto *insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
                for (auto &&stat : insSeq->stats) {
                    body.emplace_back(move(stat));
                }
                body.emplace_back(move(insSeq->expr));
            } else {
                body.emplace_back(move(block->body));
            }

            // NOTE: the code in this block _STEALS_ trees. No _return empty_'s should go after it
        }

        stats.emplace_back(ast::MK::Class(loc, loc, move(asgn->lhs), std::move(ancestors), std::move(body)));
    }

    return stats;
}

}; // namespace sorbet::rewriter
