#include "rewriter/Delegate.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include <optional>

using namespace std;

namespace sorbet::rewriter {

static bool literalSymbolEqual(const core::GlobalState &gs, ast::Expression *node, core::NameRef name) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isSymbol(gs) && lit->asSymbol(gs) == name;
    }
    return false;
}

static bool isLiteralTrue(const core::GlobalState &gs, ast::Expression *node) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isTrue(gs);
    }
    return false;
}

static optional<core::NameRef> stringOrSymbolNameRef(const core::GlobalState &gs, ast::Expression *node) {
    auto lit = ast::cast_tree<ast::Literal>(node);
    if (!lit) {
        return nullopt;
    }
    if (lit->isSymbol(gs)) {
        return lit->asSymbol(gs);
    } else if (lit->isString(gs)) {
        return lit->asString(gs);
    } else {
        return nullopt;
    }
}

vector<unique_ptr<ast::Expression>> Delegate::run(core::MutableContext ctx, const ast::Send *send) {
    vector<unique_ptr<ast::Expression>> empty;
    auto loc = send->loc;

    if (send->fun != core::Names::delegate()) {
        return empty;
    }

    if (send->args.empty()) {
        return empty;
    }

    auto options = ast::cast_tree<ast::Hash>(send->args.back().get());
    if (!options) {
        return empty;
    }

    if (send->args.size() == 1) {
        // there has to be at least one positional argument
        return empty;
    }

    ast::Expression *prefixNode = nullptr;
    core::NameRef toName;
    {
        optional<core::NameRef> to;
        for (int i = 0; i < options->keys.size(); i++) {
            if (literalSymbolEqual(ctx, options->keys[i].get(), core::Names::to())) {
                to = stringOrSymbolNameRef(ctx, options->values[i].get());
            }
            if (literalSymbolEqual(ctx, options->keys[i].get(), core::Names::prefix())) {
                prefixNode = options->values[i].get();
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
        if (isLiteralTrue(ctx, prefixNode)) {
            beforeUnderscore = toName.data(ctx)->shortName(ctx);
            useToAsPrefix = true;
        } else if (auto result = stringOrSymbolNameRef(ctx, prefixNode)) {
            beforeUnderscore = result->data(ctx)->shortName(ctx);
        } else {
            return empty;
        }
    }

    vector<unique_ptr<ast::Expression>> methodStubs;
    for (int i = 0; i < send->args.size() - 1; i++) {
        auto lit = ast::cast_tree<ast::Literal>(send->args[i].get());
        if (!lit || !lit->isSymbol(ctx)) {
            return empty;
        }
        core::NameRef methodName;
        if (prefixNode) {
            if (useToAsPrefix && (beforeUnderscore.empty() || beforeUnderscore[0] == '@')) {
                // Active Support raises at runtime for these cases
                return empty;
            }
            methodName = ctx.state.enterNameUTF8(
                fmt::format("{}_{}", beforeUnderscore, lit->asSymbol(ctx).data(ctx)->shortName(ctx)));
        } else {
            methodName = lit->asSymbol(ctx);
        }
        // sig {params(arg0: T.untyped, blk: Proc).returns(T.untyped)}
        ast::Hash::ENTRY_store paramsKeys;
        paramsKeys.emplace_back(ast::MK::Symbol(loc, core::Names::arg0()));
        paramsKeys.emplace_back(ast::MK::Symbol(loc, core::Names::blkArg()));

        ast::Hash::ENTRY_store paramsValues;
        paramsValues.emplace_back(ast::MK::Untyped(loc));
        paramsValues.emplace_back(ast::MK::Nilable(loc, ast::MK::Constant(loc, core::Symbols::Proc())));

        methodStubs.push_back(ast::MK::Sig(loc, ast::MK::Hash(loc, std::move(paramsKeys), std::move(paramsValues)),
                                           ast::MK::Untyped(loc)));

        // def $methodName(*arg0, &blk); end
        ast::MethodDef::ARGS_store args;
        args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
        args.emplace_back(std::make_unique<ast::BlockArg>(loc, ast::MK::Local(loc, core::Names::blkArg())));

        methodStubs.push_back(
            ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), methodName, std::move(args), ast::MK::EmptyTree()));
    }

    return methodStubs;
}

} // namespace sorbet::rewriter
