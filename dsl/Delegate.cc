#include "dsl/Delegate.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include <optional>

using namespace std;

namespace sorbet::dsl {

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

vector<unique_ptr<ast::Expression>> Delegate::replaceDSL(core::MutableContext ctx, const ast::Send *send) {
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
            if (useToAsPrefix && !beforeUnderscore.empty() && beforeUnderscore[0] == '@') {
                // Active Support raises at runtime for this case
                return empty;
            }
            methodName = ctx.state.enterNameUTF8(
                fmt::format("{}_{}", beforeUnderscore, lit->asSymbol(ctx).data(ctx)->shortName(ctx)));
        } else {
            methodName = lit->asSymbol(ctx);
        }
        // sig {params(arg0: T.untyped).returns(T.untyped)}
        // def $methodName(*arg0); end
        methodStubs.push_back(ast::MK::Sig1(loc, ast::MK::Symbol(loc, core::Names::arg0()), ast::MK::Untyped(loc),
                                            ast::MK::Untyped(loc)));
        methodStubs.push_back(ast::MK::Method1(loc, loc, methodName,
                                               ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())),
                                               ast::MK::EmptyTree(), ast::MethodDef::DSLSynthesized));
    }

    return methodStubs;
}

} // namespace sorbet::dsl
