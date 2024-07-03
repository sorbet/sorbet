#include "rewriter/DefDelegator.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"
#include "rewriter/Util.h"
#include <optional>

using namespace std;

namespace sorbet::rewriter {

/// Generate method stub and sig for the delegator method
void generateStub(vector<ast::ExpressionPtr> &methodStubs, const core::LocOffsets &loc,
                  const core::NameRef &methodName) {
    // sig {params(arg0: T.untyped, blk: Proc).returns(T.untyped)}
    auto sigArgs = ast::MK::SendArgs(ast::MK::Symbol(loc, core::Names::arg0()), ast::MK::Untyped(loc),
                                     ast::MK::Symbol(loc, core::Names::blkArg()),
                                     ast::MK::Nilable(loc, ast::MK::Constant(loc, core::Symbols::Proc())));

    methodStubs.push_back(ast::MK::Sig(loc, std::move(sigArgs), ast::MK::Untyped(loc)));

    // def $methodName(*arg0, &blk); end
    ast::MethodDef::ARGS_store args;
    args.emplace_back(ast::MK::RestArg(loc, ast::MK::ResolvedLocal(loc, core::Names::arg0())));
    args.emplace_back(ast::make_expression<ast::BlockArg>(loc, ast::MK::ResolvedLocal(loc, core::Names::blkArg())));

    methodStubs.push_back(
        ast::MK::SyntheticMethod(loc, loc, methodName, std::move(args), ast::MK::RaiseUnimplemented(loc)));
}

/// Handle #def_delegator for a single delegate method
vector<ast::ExpressionPtr> runDefDelegator(core::MutableContext ctx, const ast::Send *send) {
    vector<ast::ExpressionPtr> methodStubs;
    auto loc = send->loc;

    // This method takes 2..3 positional arguments and no keyword args:
    // `def_delegator(accessor, method, ali = method)`
    if (!(send->numPosArgs() == 2 || send->numPosArgs() == 3)) {
        return methodStubs;
    }

    if (send->hasKwArgs()) {
        return methodStubs;
    }

    auto *accessor = ast::cast_tree<ast::Literal>(send->getPosArg(0));
    if (!accessor || !accessor->isName()) {
        return methodStubs;
    }

    auto *method = ast::cast_tree<ast::Literal>(send->getPosArg(1));
    if (!method || !method->isSymbol()) {
        return methodStubs;
    }

    core::NameRef methodName = method->asSymbol();

    if (send->numPosArgs() == 3) {
        auto *alias = ast::cast_tree<ast::Literal>(send->getPosArg(2));
        if (!alias || !alias->isSymbol()) {
            return methodStubs;
        }

        methodName = alias->asSymbol();
    }

    generateStub(methodStubs, loc, methodName);

    // Include the original call to def_delegator so sorbet will still type-check it
    // and throw errors if the class (or its parent) didn't `extend Forwardable`
    methodStubs.push_back(send->deepCopy());

    return methodStubs;
}

/// Handle #def_delegators for zero or more delegate methods
vector<ast::ExpressionPtr> runDefDelegators(core::MutableContext ctx, const ast::Send *send) {
    vector<ast::ExpressionPtr> methodStubs;
    auto loc = send->loc;

    // This method takes 1.. positional arguments and no keyword args:
    // `def_delegators(accessor, methods*)`
    if (send->numPosArgs() == 0) {
        return methodStubs;
    }

    if (send->hasKwArgs()) {
        return methodStubs;
    }

    auto *accessor = ast::cast_tree<ast::Literal>(send->getPosArg(0));
    if (!accessor || !accessor->isName()) {
        return methodStubs;
    }

    for (int i = 1, numPosArgs = send->numPosArgs(); i < numPosArgs; ++i) {
        auto *method = ast::cast_tree<ast::Literal>(send->getPosArg(i));
        // Skip method names that we don't understand, but continue to emit
        // desugared calls for the ones we do.
        if (!method || !method->isSymbol()) {
            continue;
        }

        generateStub(methodStubs, loc, method->asSymbol());
    }

    // Include the original call to def_delegators so sorbet will still type-check it
    // and throw errors if the class (or its parent) didn't `extend Forwardable`
    methodStubs.push_back(send->deepCopy());

    return methodStubs;
}

vector<ast::ExpressionPtr> DefDelegator::run(core::MutableContext ctx, const ast::Send *send) {
    if (send->fun == core::Names::defDelegator()) {
        return runDefDelegator(ctx, send);
    } else if (send->fun == core::Names::defDelegators()) {
        return runDefDelegators(ctx, send);
    } else {
        return vector<ast::ExpressionPtr>();
    }
}

} // namespace sorbet::rewriter
