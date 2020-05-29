#include "rewriter/Struct.h"
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

// TODO this isn't quite right since the scoping will change. This would
// really all be easier if we could run after class naming :/
unique_ptr<ast::Expression> dupName(ast::Expression *node) {
    auto empty = ast::cast_tree<ast::EmptyTree>(node);
    if (empty) {
        return ast::MK::EmptyTree();
    }
    if (node->isSelfReference()) {
        return ast::MK::EmptyTree();
    }
    if (auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(node)) {
        auto newScope = dupName(cnst->scope.get());
        ENFORCE(newScope);
        return ast::MK::UnresolvedConstant(node->loc, std::move(newScope), cnst->cnst);
    } else if (auto cnst = ast::cast_tree<ast::ConstantLit>(node)) {
        return ast::MK::Constant(node->loc, cnst->symbol);
    } else {
        Exception::raise("dupName called on a node that is neither an UnresolvedConstant nor a Constant");
    }
}

static bool isKeywordInitKey(const core::GlobalState &gs, ast::Expression *node) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isSymbol(gs) && lit->asSymbol(gs) == core::Names::keywordInit();
    }
    return false;
}

static bool isLiteralTrue(const core::GlobalState &gs, ast::Expression *node) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isTrue(gs);
    }
    return false;
}

vector<unique_ptr<ast::Expression>> Struct::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<unique_ptr<ast::Expression>> empty;

    if (ctx.state.runningUnderAutogen) {
        return empty;
    }

    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
    if (lhs == nullptr) {
        return empty;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
    if (send == nullptr) {
        return empty;
    }

    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv.get());
    if (recv == nullptr) {
        return empty;
    }

    if (!ast::isa_tree<ast::EmptyTree>(recv->scope.get()) || recv->cnst != core::Symbols::Struct().data(ctx)->name ||
        send->fun != core::Names::new_() || send->args.empty()) {
        return empty;
    }

    auto loc = asgn->loc;

    ast::MethodDef::ARGS_store newArgs;
    ast::Hash::ENTRY_store sigKeys;
    ast::Hash::ENTRY_store sigValues;
    ast::ClassDef::RHS_store body;

    bool keywordInit = false;
    if (auto hash = ast::cast_tree<ast::Hash>(send->args.back().get())) {
        if (send->args.size() == 1) {
            // leave bad usages like `Struct.new(keyword_init: true)` untouched so we error later
            return empty;
        }
        if (hash->keys.size() != 1) {
            return empty;
        }
        auto key = hash->keys.front().get();
        auto value = hash->values.front().get();
        if (isKeywordInitKey(ctx, key) && isLiteralTrue(ctx, value)) {
            keywordInit = true;
        }
    }

    const auto n = keywordInit ? send->args.size() - 1 : send->args.size();
    for (int i = 0; i < n; i++) {
        auto sym = ast::cast_tree<ast::Literal>(send->args[i].get());
        if (!sym || !sym->isSymbol(ctx)) {
            return empty;
        }
        core::NameRef name = sym->asSymbol(ctx);
        auto symLoc = sym->loc;
        if (symLoc.exists() && absl::StartsWith(core::Loc(ctx.file, symLoc).source(ctx), ":")) {
            symLoc = core::LocOffsets{symLoc.beginPos() + 1, symLoc.endPos()};
        }

        sigKeys.emplace_back(ast::MK::Symbol(symLoc, name));
        sigValues.emplace_back(ast::MK::Constant(symLoc, core::Symbols::BasicObject()));
        unique_ptr<ast::Reference> argName = ast::MK::Local(symLoc, name);
        if (keywordInit) {
            argName = ast::MK::KeywordArg(symLoc, move(argName));
        }
        newArgs.emplace_back(ast::MK::OptionalArg(symLoc, move(argName), ast::MK::Nil(symLoc)));

        body.emplace_back(
            ast::MK::SyntheticMethod0(symLoc, core::Loc(ctx.file, symLoc), name, ast::MK::RaiseUnimplemented(loc)));
        body.emplace_back(ast::MK::SyntheticMethod1(symLoc, core::Loc(ctx.file, symLoc), name.addEq(ctx),
                                                    ast::MK::Local(symLoc, name), ast::MK::RaiseUnimplemented(loc)));
    }

    // Elem = type_member(fixed: T.untyped)
    body.emplace_back(ast::MK::Assign(
        loc, ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), core::Names::Constants::Elem()),
        ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::typeMember(),
                       ast::MK::Hash1(loc, ast::MK::Symbol(loc, core::Names::fixed()), ast::MK::Untyped(loc)))));

    if (send->block != nullptr) {
        // Steal the trees, because the run is going to remove the original send node from the tree anyway.
        if (auto insSeq = ast::cast_tree<ast::InsSeq>(send->block->body.get())) {
            for (auto &&stat : insSeq->stats) {
                body.emplace_back(move(stat));
            }
            body.emplace_back(move(insSeq->expr));
        } else {
            body.emplace_back(move(send->block->body));
        }

        // NOTE: the code in this block _STEALS_ trees. No _return empty_'s should go after it
    }

    body.emplace_back(ast::MK::SigVoid(loc, ast::MK::Hash(loc, std::move(sigKeys), std::move(sigValues))));
    body.emplace_back(ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), core::Names::initialize(),
                                               std::move(newArgs), ast::MK::RaiseUnimplemented(loc)));

    ast::ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(ast::MK::UnresolvedConstant(loc, ast::MK::Constant(loc, core::Symbols::root()),
                                                       core::Names::Constants::Struct()));

    vector<unique_ptr<ast::Expression>> stats;
    stats.emplace_back(
        ast::MK::Class(loc, core::Loc(ctx.file, loc), std::move(asgn->lhs), std::move(ancestors), std::move(body)));
    return stats;
}

}; // namespace sorbet::rewriter
