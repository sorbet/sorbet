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
    ast::TreePtr *prevStat = nullptr;
    UnorderedMap<void *, vector<ast::TreePtr>> replaceNodes;
    for (auto &stat : cdef->rhs) {
        if (auto send = ast::cast_tree<ast::Send>(stat)) {
            // we only care about sends if they're `module_function`
            if (send->fun == core::Names::moduleFunction() && send->recv->isSelfReference()) {
                if (send->args.size() == 0) {
                    // a `module_function` with no args changes the way that every subsequent method definition works so
                    // we set this flag so we know that the rest of the defns should be rewritten
                    moduleFunctionActive = true;
                    // putting in an empty statement list, which means we remove this statement when we get around to
                    // updating the statement list
                    vector<ast::TreePtr> empty;
                    replaceNodes[stat.get()] = move(empty);
                } else {
                    // if we do have arguments, then we can rewrite them appropriately
                    replaceNodes[stat.get()] = run(ctx, send, prevStat);
                }
            }
        } else if (auto defn = ast::cast_tree<ast::MethodDef>(stat)) {
            // if we've already seen a bare `module_function` call, then every subsequent method definition needs to get
            // rewritten appropriately
            if (moduleFunctionActive) {
                auto res = rewriteDefn(ctx, stat, prevStat);
                // we might not actually want to rewrite this definition (e.g. if it's already a static method) so this
                // check is needed
                if (res.size() != 0) {
                    replaceNodes[stat.get()] = move(res);
                }
            }
        }
        // this is to give us access to the `sig`
        prevStat = &stat;
    }

    // now we clear out the definitions of the class...
    auto oldRHS = std::move(cdef->rhs);
    cdef->rhs.clear();
    cdef->rhs.reserve(oldRHS.size());

    // and either put them back, or replace them
    for (auto &stat : oldRHS) {
        auto replacement = replaceNodes.find(stat.get());
        if (replacement == replaceNodes.end()) {
            cdef->rhs.emplace_back(std::move(stat));
        } else {
            for (auto &newNode : replacement->second) {
                cdef->rhs.emplace_back(std::move(newNode));
            }
        }
    }
}

vector<ast::TreePtr> ModuleFunction::rewriteDefn(core::MutableContext ctx, const ast::TreePtr &expr,
                                                 const ast::TreePtr *prevStat) {
    vector<ast::TreePtr> stats;
    auto mdef = ast::cast_tree_const<ast::MethodDef>(expr);
    // only do this rewrite to method defs that aren't self methods
    if (mdef == nullptr || mdef->flags.isSelfMethod) {
        stats.emplace_back(expr->deepCopy());
        return stats;
    }

    auto sig = ast::cast_tree_const<ast::Send>(*prevStat);
    bool hasSig = sig && sig->fun == core::Names::sig();
    auto loc = expr->loc;

    // this creates a private copy of the method
    auto privateCopy = expr->deepCopy();
    stats.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::private_(), move(privateCopy)));

    // as well as a public static copy of the method
    if (hasSig) {
        stats.emplace_back(sig->deepCopy());
    }
    auto moduleCopy = expr->deepCopy();
    ENFORCE(moduleCopy, "Should be non-nil.");
    auto *newDefn = ast::cast_tree<ast::MethodDef>(moduleCopy);
    newDefn->flags.isSelfMethod = true;
    newDefn->flags.isRewriterSynthesized = true;
    stats.emplace_back(move(moduleCopy));

    return stats;
}

vector<ast::TreePtr> ModuleFunction::run(core::MutableContext ctx, ast::Send *send, const ast::TreePtr *prevStat) {
    vector<ast::TreePtr> stats;

    if (send->fun != core::Names::moduleFunction()) {
        return stats;
    }

    for (auto &arg : send->args) {
        if (ast::isa_tree<ast::MethodDef>(arg)) {
            return ModuleFunction::rewriteDefn(ctx, arg, prevStat);
        } else if (auto lit = ast::cast_tree<ast::Literal>(arg)) {
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
                    if (auto e = ctx.beginError(lit->loc, core::errors::Rewriter::BadModuleFunction)) {
                        e.setHeader("Bad attribute name \"{}\"", absl::CEscape(shortName));
                    }
                }
            }

            stats.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::private_(), lit->deepCopy()));
            ast::MethodDef::ARGS_store args;
            args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
            args.emplace_back(ast::make_tree<ast::BlockArg>(loc, ast::MK::Local(loc, core::Names::blkArg())));
            auto methodDef = ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), methodName, std::move(args),
                                                      ast::MK::EmptyTree());
            ast::cast_tree_nonnull<ast::MethodDef>(methodDef).flags.isSelfMethod = true;
            stats.emplace_back(std::move(methodDef));
        } else {
            if (auto e = ctx.beginError(arg->loc, core::errors::Rewriter::BadModuleFunction)) {
                e.setHeader("Bad argument to `{}`: must be a symbol, string, method definition, or nothing",
                            "module_function");
            }
        }
    }

    return stats;
}
} // namespace sorbet::rewriter
