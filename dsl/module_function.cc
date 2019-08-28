#include "dsl/module_function.h"
#include "absl/strings/escaping.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

bool ModuleFunction::isModuleFunction(const ast::Expression *expr) {
    if (auto send = ast::cast_tree_const<ast::Send>(expr)) {
        return send->fun == core::Names::moduleFunction();
    }
    return false;
}

vector<unique_ptr<ast::Expression>> ModuleFunction::rewriteDefn(core::MutableContext ctx, const ast::Expression *expr,
                                                                const ast::Expression *prevStat, bool usedSig) {
    vector<unique_ptr<ast::Expression>> stats;
    auto mdef = ast::cast_tree_const<ast::MethodDef>(expr);
    // only do this rewrite to method defs that aren't self methods
    if (mdef == nullptr || (mdef->flags & ast::MethodDef::SelfMethod) != 0) {
        stats.emplace_back(expr->deepCopy());
        return stats;
    }

    auto sig = ast::cast_tree_const<ast::Send>(prevStat);
    bool hasSig = sig && sig->fun == core::Names::sig();
    auto loc = expr->loc;

    // copy the sig first
    if (hasSig && !usedSig) {
        stats.emplace_back(sig->deepCopy());
    }

    // this creates a private copy of the method
    unique_ptr<ast::Expression> privateCopy = expr->deepCopy();
    stats.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::private_(), move(privateCopy)));

    // as well as a public static copy of the method
    if (hasSig) {
        stats.emplace_back(sig->deepCopy());
    }
    unique_ptr<ast::Expression> moduleCopy = expr->deepCopy();
    ENFORCE(moduleCopy, "Should be non-nil.");
    auto newDefn = ast::cast_tree<ast::MethodDef>(moduleCopy.get());
    newDefn->flags |= ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized;
    stats.emplace_back(move(moduleCopy));

    return stats;
}

vector<unique_ptr<ast::Expression>> ModuleFunction::replaceDSL(core::MutableContext ctx, ast::Send *send,
                                                               const ast::Expression *prevStat) {
    vector<unique_ptr<ast::Expression>> stats;

    if (send->fun != core::Names::moduleFunction()) {
        return stats;
    }

    // if module_function is used in a context with no args, then the rest of the definitions after this one should be
    // rewritten, so let's return the nullary call to module_function as a "replacement" expression: the DSL loop will
    // in turn know to apply a rewrite to all subsequent definitions
    if (send->args.size() == 0) {
        stats.emplace_back(send->deepCopy());
    }

    for (auto &arg : send->args) {
        if (ast::isa_tree<ast::MethodDef>(arg.get())) {
            return ModuleFunction::rewriteDefn(ctx, arg.get(), prevStat, true);
        } else if (auto lit = ast::cast_tree<ast::Literal>(arg.get())) {
            core::NameRef methodName = core::NameRef::noName();
            auto loc = send->loc;
            if (lit->isSymbol(ctx)) {
                methodName = lit->asSymbol(ctx);
            } else if (lit->isString(ctx)) {
                core::NameRef nameRef = lit->asString(ctx);
                auto shortName = nameRef.data(ctx)->shortName(ctx);
                bool validAttr = (isalpha(shortName.front()) || shortName.front() == '_') &&
                                 absl::c_all_of(shortName, [](char c) { return isalnum(c) || c == '_'; });
                if (validAttr) {
                    methodName = nameRef;
                } else {
                    if (auto e = ctx.state.beginError(lit->loc, core::errors::DSL::BadModuleFunction)) {
                        e.setHeader("Bad attribute name \"{}\"", absl::CEscape(shortName));
                    }
                }
            }

            stats.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::private_(), lit->deepCopy()));
            ast::MethodDef::ARGS_store args;
            args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
            args.emplace_back(std::make_unique<ast::BlockArg>(loc, ast::MK::Local(loc, core::Names::blkArg())));
            stats.emplace_back(ast::MK::Method(loc, loc, methodName, std::move(args), ast::MK::EmptyTree(),
                                               ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized));
        } else {
            if (auto e = ctx.state.beginError(arg->loc, core::errors::DSL::BadModuleFunction)) {
                e.setHeader("Bad argument to `{}`: must be a symbol, string, or method definition", "module_function");
            }
        }
    }

    return stats;
}
} // namespace sorbet::dsl
