#include "rewriter/ModuleFunction.h"
#include "absl/strings/escaping.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

void ModuleFunction::run(core::MutableContext ctx, ast::ClassDef *cdef) {
    // once we see a bare `module_function`, we should replace every subsequent definition
    bool moduleFunctionActive = false;
    ast::Expression *prevStat = nullptr;
    UnorderedMap<ast::Expression *, vector<unique_ptr<ast::Expression>>> replaceNodes;
    for (auto &stat : cdef->rhs) {
        if (auto send = ast::cast_tree<ast::Send>(stat.get())) {
            // we only care about sends if they're `module_function`
            if (send->fun == core::Names::moduleFunction() && send->recv->isSelfReference()) {
                if (send->args.size() == 0) {
                    // a `module_function` with no args changes the way that every subsequent method definition works so
                    // we set this flag so we know that the rest of the defns should be rewritten
                    moduleFunctionActive = true;
                    // putting in an empty statement list, which means we remove this statement when we get around to
                    // updating the statement list
                    vector<unique_ptr<ast::Expression>> empty;
                    replaceNodes[stat.get()] = move(empty);
                } else {
                    // if we do have arguments, then we can rewrite them appropriately
                    replaceNodes[stat.get()] = run(ctx, send, prevStat);
                }
            }
        } else if (auto defn = ast::cast_tree<ast::MethodDef>(stat.get())) {
            // if we've already seen a bare `module_function` call, then every subsequent method definition needs to get
            // rewritten appropriately
            if (moduleFunctionActive) {
                auto res = rewriteDefn(ctx, defn, prevStat);
                // we might not actually want to rewrite this definition (e.g. if it's already a static method) so this
                // check is needed
                if (res.size() != 0) {
                    replaceNodes[stat.get()] = move(res);
                }
            }
        }
        // this is to give us access to the `sig`
        prevStat = stat.get();
    }

    // now we clear out the definitions of the class...
    auto oldRHS = std::move(cdef->rhs);
    cdef->rhs.clear();
    cdef->rhs.reserve(oldRHS.size());

    // and either put them back, or replace them
    for (auto &stat : oldRHS) {
        if (replaceNodes.find(stat.get()) == replaceNodes.end()) {
            cdef->rhs.emplace_back(std::move(stat));
        } else {
            for (auto &newNode : replaceNodes.at(stat.get())) {
                cdef->rhs.emplace_back(std::move(newNode));
            }
        }
    }
}

vector<unique_ptr<ast::Expression>> ModuleFunction::rewriteDefn(core::MutableContext ctx, const ast::Expression *expr,
                                                                const ast::Expression *prevStat) {
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
    newDefn->flags |= ast::MethodDef::SelfMethod | ast::MethodDef::RewriterSynthesized;
    stats.emplace_back(move(moduleCopy));

    return stats;
}

vector<unique_ptr<ast::Expression>> ModuleFunction::run(core::MutableContext ctx, ast::Send *send,
                                                        const ast::Expression *prevStat) {
    vector<unique_ptr<ast::Expression>> stats;

    if (send->fun != core::Names::moduleFunction()) {
        return stats;
    }

    for (auto &arg : send->args) {
        if (ast::isa_tree<ast::MethodDef>(arg.get())) {
            return ModuleFunction::rewriteDefn(ctx, arg.get(), prevStat);
        } else if (auto lit = ast::cast_tree<ast::Literal>(arg.get())) {
            core::NameRef methodName;
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
                    if (auto e = ctx.state.beginError(lit->loc, core::errors::Rewriter::BadModuleFunction)) {
                        e.setHeader("Bad attribute name \"{}\"", absl::CEscape(shortName));
                    }
                }
            }

            stats.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::private_(), lit->deepCopy()));
            ast::MethodDef::ARGS_store args;
            args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
            args.emplace_back(std::make_unique<ast::BlockArg>(loc, ast::MK::Local(loc, core::Names::blkArg())));
            auto methodDef = ast::MK::Method(loc, loc, methodName, std::move(args), ast::MK::EmptyTree());
            methodDef->flags |= ast::MethodDef::Flags::SelfMethod;
            stats.emplace_back(std::move(methodDef));
        } else {
            if (auto e = ctx.state.beginError(arg->loc, core::errors::Rewriter::BadModuleFunction)) {
                e.setHeader("Bad argument to `{}`: must be a symbol, string, method definition, or nothing",
                            "module_function");
            }
        }
    }

    return stats;
}
} // namespace sorbet::rewriter
