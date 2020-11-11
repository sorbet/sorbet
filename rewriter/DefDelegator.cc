#include "rewriter/DefDelegator.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "rewriter/Util.h"
#include <optional>

using namespace std;

namespace sorbet::rewriter {

vector<ast::TreePtr> DefDelegator::run(core::MutableContext ctx, const ast::Send *send) {
    vector<ast::TreePtr> empty;
    auto loc = send->loc;

    if (send->fun != core::Names::defDelegator()) {
        return empty;
    }

    // This method takes 2..3 positional arguments and no keyword args:
    // `def_delegator(accessor, method, ali = method)`
    if (!(send->numPosArgs == 2 || send->numPosArgs == 3)) {
        return empty;
    }

    if (send->hasKwArgs()) {
        return empty;
    }

    auto *accessor = ast::cast_tree<ast::Literal>(send->args[0]);
    auto *method = ast::cast_tree<ast::Literal>(send->args[1]);

    if (!accessor || !(accessor->isSymbol(ctx) || accessor->isString(ctx))) {
        return empty;
    }

    if (!method || !method->isSymbol(ctx)) {
        return empty;
    }

    core::NameRef methodName = method->asSymbol(ctx);

    if (send->numPosArgs == 3) {
        auto *alias = ast::cast_tree<ast::Literal>(send->args[2]);

        if (!alias || !alias->isSymbol(ctx)) {
            return empty;
        }

        methodName = alias->asSymbol(ctx);
    }

    vector<ast::TreePtr> methodStubs;

    // sig {params(arg0: T.untyped, blk: Proc).returns(T.untyped)}
    ast::Send::ARGS_store sigArgs;
    sigArgs.emplace_back(ast::MK::Symbol(loc, core::Names::arg0()));
    sigArgs.emplace_back(ast::MK::Untyped(loc));

    sigArgs.emplace_back(ast::MK::Symbol(loc, core::Names::blkArg()));
    sigArgs.emplace_back(ast::MK::Nilable(loc, ast::MK::Constant(loc, core::Symbols::Proc())));

    methodStubs.push_back(ast::MK::Sig(loc, std::move(sigArgs), ast::MK::Untyped(loc)));

    // def $methodName(*arg0, &blk); end
    ast::MethodDef::ARGS_store args;
    args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
    args.emplace_back(ast::make_tree<ast::BlockArg>(loc, ast::MK::Local(loc, core::Names::blkArg())));

    methodStubs.push_back(ast::MK::SyntheticMethod(loc, loc, methodName, std::move(args), ast::MK::EmptyTree()));

    return methodStubs;
}

} // namespace sorbet::rewriter
