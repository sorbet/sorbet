#include "rewriter/Delegate.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "rewriter/util/Util.h"
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

    // Backwards compatibility: by default, Sorbet rewrites `delegate` into stubs returning `T.untyped`.
    // We only emit extra metadata for return type inference behind an experimental flag.
    const bool delegateReturnTypesEnabled = ctx.state.cacheSensitiveOptions.delegateReturnTypesEnabled;

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
    bool allowNil = false;
    core::NameRef toName;
    {
        optional<core::NameRef> to;
        const auto allowNilName = ctx.state.enterNameUTF8("allow_nil");
        for (auto [key, val] : options->kviter()) {
            if (literalSymbolEqual(ctx, key, core::Names::to())) {
                to = stringOrSymbolNameRef(ctx, val);
            }
            if (literalSymbolEqual(ctx, key, core::Names::prefix())) {
                prefixNode = &val;
            }
            if (literalSymbolEqual(ctx, key, allowNilName)) {
                // ActiveSupport accepts allow_nil: true/false. We only model literal booleans.
                if (auto lit = ast::cast_tree<ast::Literal>(val)) {
                    if (lit->isTrue(ctx)) {
                        allowNil = true;
                    } else if (lit->isFalse(ctx)) {
                        allowNil = false;
                    }
                }
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
    for (auto &arg : send->posArgs()) {
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isSymbol()) {
            return empty;
        }
        const core::NameRef delegatedMethodName = lit->asSymbol();
        core::NameRef methodName;
        if (prefixNode) {
            if (useToAsPrefix && (beforeUnderscore.empty() || beforeUnderscore[0] == '@')) {
                // Active Support raises at runtime for these cases
                return empty;
            }
            methodName =
                ctx.state.enterNameUTF8(fmt::format("{}_{}", beforeUnderscore, delegatedMethodName.shortName(ctx)));
        } else {
            methodName = delegatedMethodName;
        }
        // sig {params(arg0: T.untyped, blk: Proc).returns(T.untyped)}
        auto sigArgs = ast::MK::SendArgs(ast::MK::Symbol(loc, core::Names::arg0()), ast::MK::Untyped(loc),
                                         ast::MK::Symbol(loc, core::Names::blkArg()),
                                         ast::MK::Nilable(loc, ast::MK::Constant(loc, core::Symbols::Proc())));

        methodStubs.push_back(ast::MK::Sig(loc, std::move(sigArgs), ast::MK::Untyped(loc)));

        // def $methodName(*arg0, &blk); end
        ast::MethodDef::PARAMS_store params;
        params.emplace_back(ast::MK::RestParam(loc, ast::MK::Local(loc, core::Names::arg0())));
        params.emplace_back(ast::make_expression<ast::BlockParam>(loc, ast::MK::Local(loc, core::Names::blkArg())));

        ast::ExpressionPtr rhs;
        if (delegateReturnTypesEnabled) {
            // Marker body consumed by a resolver post-pass (only when the flag is enabled).
            // The call target is untyped, so this marker never produces type errors itself.
            auto markerFun = ctx.state.enterNameUTF8("__sorbet_delegate_stub__");
            auto markerArgs = ast::MK::SendArgs(ast::MK::Symbol(loc, toName), ast::MK::Symbol(loc, delegatedMethodName),
                                                allowNil ? ast::MK::True(loc) : ast::MK::False(loc));
            rhs = ast::MK::Send(loc, ast::MK::UntypedNil(loc), markerFun, loc, 3, std::move(markerArgs));
        } else {
            rhs = ast::MK::EmptyTree();
        }

        methodStubs.push_back(ast::MK::SyntheticMethod(loc, loc, methodName, std::move(params), std::move(rhs)));
    }

    return methodStubs;
}

} // namespace sorbet::rewriter
