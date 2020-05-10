#include "rewriter/DefaultArgs.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "common/typecase.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

ast::TreePtr mangleSig(core::Context ctx, ast::TreePtr expr, ast::TreePtr &param) {
    auto sig = ast::cast_tree<ast::Send>(expr);
    ENFORCE(sig);
    ENFORCE(sig->fun == core::Names::sig());

    ast::UnresolvedIdent *ident = nullptr;
    if (auto *kw = ast::cast_tree<ast::KeywordArg>(param)) {
        ident = ast::cast_tree<ast::UnresolvedIdent>(kw->expr);
    } else {
        ident = ast::cast_tree<ast::UnresolvedIdent>(param);
    }

    if (!ident) {
        return ast::MK::EmptyTree();
    }
    auto name = ident->name;

    ast::TreePtr retType;

    if (sig->block == nullptr) {
        return ast::MK::EmptyTree();
    }

    auto &sigBlock = ast::ref_tree<ast::Block>(sig->block);
    auto send = ast::cast_tree<ast::Send>(sigBlock.body);
    if (!send) {
        return ast::MK::EmptyTree();
    }

    while (send != nullptr) {
        switch (send->fun._id) {
            case core::Names::params()._id: {
                if (send->args.size() != 1) {
                    return ast::MK::EmptyTree();
                }
                auto *hash = ast::cast_tree<ast::Hash>(send->args[0]);
                if (!hash) {
                    return ast::MK::EmptyTree();
                }
                int i = -1;
                for (auto &key : hash->keys) {
                    i++;
                    auto &value = hash->values[i];
                    auto lit = ast::cast_tree<ast::Literal>(key);
                    if (lit && lit->isSymbol(ctx)) {
                        auto symName = lit->asSymbol(ctx);
                        if (name == symName) {
                            retType = value->deepCopy();
                        }
                    }
                }
                break;
            }

            case core::Names::abstract()._id: {
                // Don't make this method at all since abstract methods can't
                // have bodies
                return nullptr;
            }

            case core::Names::override_()._id: {
                // A total hack but we allow .void.void or .void.returns and
                // the one with content wins
                send->fun = core::Names::void_();
            }
        }
        auto recv = ast::cast_tree<ast::Send>(send->recv);
        send = recv;
    }

    send = ast::cast_tree<ast::Send>(sigBlock.body);
    while (send != nullptr) {
        switch (send->fun._id) {
            case core::Names::returns()._id: {
                if (!retType) {
                    return ast::MK::EmptyTree();
                }
                send->args[0] = move(retType);
                break;
            }

            case core::Names::void_()._id: {
                if (!retType) {
                    return ast::MK::EmptyTree();
                }
                send->fun = core::Names::returns();
                send->args.emplace_back(move(retType));
                break;
            }
        }

        auto recv = ast::cast_tree<ast::Send>(send->recv);
        send = recv;
    }
    return expr;
}

ast::TreePtr dupRef(ast::TreePtr &arg) {
    ast::TreePtr newArg = nullptr;
    typecase(
        *arg, [&](ast::UnresolvedIdent *nm) { newArg = ast::MK::Local(arg->loc, nm->name); },
        [&](ast::RestArg *rest) { newArg = ast::MK::RestArg(arg->loc, dupRef(rest->expr)); },
        [&](ast::KeywordArg *kw) { newArg = ast::MK::KeywordArg(arg->loc, dupRef(kw->expr)); },
        [&](ast::OptionalArg *opt) {
            newArg = ast::MK::OptionalArg(arg->loc, dupRef(opt->expr), ast::MK::EmptyTree());
        },
        [&](ast::BlockArg *blk) { newArg = ast::MK::BlockArg(arg->loc, dupRef(blk->expr)); },
        [&](ast::ShadowArg *shadow) { newArg = ast::MK::ShadowArg(arg->loc, dupRef(shadow->expr)); });
    return newArg;
}

} // namespace

void DefaultArgs::run(core::MutableContext ctx, ast::ClassDef *klass) {
    vector<ast::TreePtr> newMethods;
    ast::Send *lastSig = nullptr;
    bool isOverload = false;

    for (auto &stat : klass->rhs) {
        typecase(
            stat.get(),
            [&](ast::Send *send) {
                if (send->fun != core::Names::sig()) {
                    return;
                }
                if (lastSig != nullptr) {
                    isOverload = true;
                    return;
                }
                lastSig = send;
            },
            [&](ast::MethodDef *mdef) {
                if (isOverload) {
                    // Overloaded methods have multiple signatures, not all of
                    // which include all the arguments. Programatically copying
                    // them over and figuring out which ones apply to which
                    // defaults and how is super hard. This is one of the
                    // reasons we don't let users write them, and only have them
                    // in the stdlib.
                    return;
                }
                auto i = -1;
                auto uniqueNum = 1;
                for (auto &methodArg : mdef->args) {
                    ++i;
                    auto arg = ast::cast_tree<ast::OptionalArg>(methodArg);
                    if (!arg) {
                        continue;
                    }

                    ENFORCE(ast::isa_tree<ast::UnresolvedIdent>(arg->expr) ||
                            ast::isa_tree<ast::KeywordArg>(arg->expr));
                    auto name = ctx.state.freshNameUnique(core::UniqueNameKind::DefaultArg, mdef->name, uniqueNum++);
                    ast::MethodDef::ARGS_store args;
                    for (auto &arg : mdef->args) {
                        args.emplace_back(dupRef(arg));
                    }
                    auto loc = arg->default_->loc;
                    auto rhs = move(arg->default_);
                    arg->default_ = ast::MK::EmptyTree();

                    if (lastSig) {
                        auto sig = mangleSig(ctx, lastSig->deepCopy(), arg->expr);
                        if (sig == nullptr) {
                            continue;
                        }
                        newMethods.emplace_back(move(sig));
                    }
                    auto defaultArgDef =
                        ast::MK::SyntheticMethod(loc, core::Loc(ctx.file, loc), name, std::move(args), std::move(rhs));
                    {
                        auto &defaultDef = ast::ref_tree<ast::MethodDef>(defaultArgDef);
                        defaultDef.flags.isSelfMethod = mdef->flags.isSelfMethod;
                    }
                    newMethods.emplace_back(move(defaultArgDef));
                }
                lastSig = nullptr;
            },

            [&](ast::Expression *expr) {});
    }

    for (auto &stat : newMethods) {
        klass->rhs.emplace_back(move(stat));
    }
}

} // namespace sorbet::rewriter
