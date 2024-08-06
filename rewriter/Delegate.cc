#include "rewriter/Delegate.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "rewriter/Util.h"
#include <optional>

using namespace std;

namespace sorbet::rewriter {

static bool literalSymbolEqual(const core::GlobalState &gs, const ast::ExpressionPtr &node, core::NameRef name) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isSymbol() && lit->asSymbol() == name;
    }
    return false;
}

static bool isLiteralTrue(const core::GlobalState &gs, const ast::ExpressionPtr &node) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isTrue(gs);
    }
    return false;
}

static optional<core::NameRef> stringOrSymbolNameRef(const core::GlobalState &gs, const ast::ExpressionPtr &node) {
    auto lit = ast::cast_tree<ast::Literal>(node);
    if (!lit || !lit->isName()) {
        return nullopt;
    }

    return lit->asName();
}

vector<ast::ExpressionPtr> Delegate::run(core::MutableContext ctx, const ast::Send *send) {
    vector<ast::ExpressionPtr> empty;
    auto loc = send->loc;

    if (send->fun != core::Names::delegate()) {
        return empty;
    }

    if (!send->hasPosArgs()) {
        // there has to be at least one positional argument
        return empty;
    }

    auto optionsTree = ASTUtil::mkKwArgsHash(send);
    auto options = ast::cast_tree<ast::Hash>(optionsTree);
    if (!options) {
        return empty;
    }

    ast::ExpressionPtr const *prefixNode = nullptr;
    core::NameRef toName;
    {
        optional<core::NameRef> to;
        for (int i = 0; i < options->keys.size(); i++) {
            if (literalSymbolEqual(ctx, options->keys[i], core::Names::to())) {
                to = stringOrSymbolNameRef(ctx, options->values[i]);
            }
            if (literalSymbolEqual(ctx, options->keys[i], core::Names::prefix())) {
                prefixNode = &options->values[i];
            }
        }

        if (to) {
            toName = *to;
        } else {
            return empty;
        }
    }

    string beforeUnderscore;
    bool useToAsPrefix = false;
    if (prefixNode) {
        if (isLiteralTrue(ctx, *prefixNode)) {
            beforeUnderscore = toName.shortName(ctx);
            useToAsPrefix = true;
        } else if (auto result = stringOrSymbolNameRef(ctx, *prefixNode)) {
            beforeUnderscore = result->shortName(ctx);
        } else {
            return empty;
        }
    }

    vector<ast::ExpressionPtr> methodStubs;
    for (int i = 0; i < send->numPosArgs(); i++) {
        auto *lit = ast::cast_tree<ast::Literal>(send->getPosArg(i));
        if (!lit || !lit->isSymbol()) {
            return empty;
        }
        core::NameRef methodName;
        if (prefixNode) {
            if (useToAsPrefix && (beforeUnderscore.empty() || beforeUnderscore[0] == '@')) {
                // Active Support raises at runtime for these cases
                return empty;
            }
            methodName =
                ctx.state.enterNameUTF8(fmt::format("{}_{}", beforeUnderscore, lit->asSymbol().shortName(ctx)));
        } else {
            methodName = lit->asSymbol();
        }
        // sig {params(arg0: T.untyped, blk: Proc).returns(T.untyped)}
        auto sigArgs = ast::MK::SendArgs(ast::MK::Symbol(loc, core::Names::arg0()), ast::MK::Untyped(loc),
                                         ast::MK::Symbol(loc, core::Names::blkArg()),
                                         ast::MK::Nilable(loc, ast::MK::Constant(loc, core::Symbols::Proc())));

        methodStubs.push_back(ast::MK::Sig(loc, std::move(sigArgs), ast::MK::Untyped(loc)));

        // def $methodName(*arg0, &blk); end
        ast::MethodDef::ARGS_store args;
        args.emplace_back(ast::MK::RestArg(loc, ast::MK::ResolvedLocal(loc, core::Names::arg0())));
        args.emplace_back(ast::make_expression<ast::BlockArg>(loc, ast::MK::ResolvedLocal(loc, core::Names::blkArg())));

        methodStubs.push_back(ast::MK::SyntheticMethod(loc, loc, methodName, std::move(args), ast::MK::EmptyTree()));
    }

    return methodStubs;
}

} // namespace sorbet::rewriter
