#include "dsl/DefaultArgs.h"
#include "ast/Helpers.h"
#include "common/typecase.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::dsl {

unique_ptr<ast::Expression> mangleSig(unique_ptr<ast::Expression> sig, int param) {
    // TODO change the return type to be that of the argument and then return
    // the sig for that
    return nullptr;
}

void DefaultArgs::patchDSL(core::MutableContext ctx, ast::ClassDef *klass) {
    vector<unique_ptr<ast::Expression>> newMethods;

    for (auto &stat : klass->rhs) {
        ast::Send *lastSig = nullptr;

        typecase(
            stat.get(),
            [&](ast::Send *send) {
                if (send->fun != core::Names::sig()) {
                    return;
                }
                lastSig = send;
            },
            [&](ast::MethodDef *mdef) {
                auto i = -1;
                auto uniqueNum = 1;
                for (auto &methodArg : mdef->args) {
                    ++i;
                    auto arg = ast::cast_tree<ast::OptionalArg>(methodArg.get());
                    if (!arg) {
                        continue;
                    }

                    ENFORCE(ast::isa_tree<ast::UnresolvedIdent>(arg->expr.get()) ||
                            ast::isa_tree<ast::KeywordArg>(arg->expr.get()));
                    auto name = ctx.state.freshNameUnique(core::UniqueNameKind::DefaultArg, mdef->name, uniqueNum++);
                    ast::MethodDef::ARGS_store args;
                    for (auto &marg : mdef->args) {
                        auto newArg = marg->deepCopy();
                        auto optArg = ast::cast_tree<ast::OptionalArg>(newArg.get());
                        if (optArg) {
                            optArg->default_ = ast::MK::EmptyTree();
                        }
                        args.emplace_back(move(newArg));
                    }
                    auto loc = arg->default_->loc;
                    auto rhs = move(arg->default_);
                    arg->default_ = ast::MK::EmptyTree();

                    if (lastSig) {
                        auto sig = mangleSig(lastSig->deepCopy(), i);
                        newMethods.emplace_back(move(sig));
                        lastSig = nullptr;
                    }
                    newMethods.emplace_back(
                        ast::MK::Method(loc, loc, name, std::move(args), std::move(rhs), mdef->flags));
                }
            },

            [&](ast::Expression *expr) {});
    }

    for (auto &stat : newMethods) {
        klass->rhs.emplace_back(move(stat));
    }
}

} // namespace sorbet::dsl
