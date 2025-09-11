#include "rewriter/Concern.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool doesExtendConcern(core::MutableContext ctx, ast::ClassDef *klass) {
    bool returnValue = false;
    for (auto &stat : klass->rhs) {
        typecase(
            stat,
            [&](ast::Send &send) {
                if (send.fun != core::Names::extend()) {
                    return;
                }
                if (!send.hasPosArgs()) {
                    return;
                }
                auto firstArg = ast::cast_tree<ast::UnresolvedConstantLit>(send.getPosArg(0));
                if (firstArg == nullptr) {
                    return;
                }
                auto firstArgScope = ast::cast_tree<ast::UnresolvedConstantLit>(firstArg->scope);
                if (firstArgScope == nullptr) {
                    return;
                }
                auto outerScope = ast::cast_tree<ast::UnresolvedConstantLit>(firstArgScope->scope);
                if (outerScope != nullptr) {
                    return;
                }
                if (firstArg->cnst != core::Names::Constants::Concern() ||
                    firstArgScope->cnst != core::Names::Constants::ActiveSupport()) {
                    return;
                }

                returnValue = true;
            },
            [&](const ast::ExpressionPtr &e) {});

        if (returnValue) {
            break;
        }
    }

    return returnValue;
}

} // namespace

// For a given ClassDef node the rewriter traverses the members (rhs) to identify `class_methods` calls
// or `module ClassMethods` definitions. For the former it transforms it into Ruby code that Sorbet can understand.
// It also handles cases where both `class_methods` and `module ClassMethods` are used.
// Knowledge of `class_methods` and its source code will help you understand the steps taken here.
// See https://api.rubyonrails.org/classes/ActiveSupport/Concern.html#method-i-class_methods.
void Concern::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
        return;
    }
    if (!doesExtendConcern(ctx, klass)) {
        return;
    }

    vector<ast::ExpressionPtr> stats;
    ast::ExpressionPtr classMethodsNode;
    for (auto &stat : klass->rhs) {
        if (auto send = ast::cast_tree<ast::Send>(stat)) {
            if (send->fun == core::Names::classMethods()) { // class_methods do ... end
                if (!send->hasBlock()) {
                    continue;
                }
                if (!send->recv.isSelfReference()) {
                    continue;
                }
                if (classMethodsNode) {
                    // ClassMethods module already exists. Let's add the block as one of its members
                    auto *block = send->block();
                    auto classDef = ast::cast_tree<ast::ClassDef>(classMethodsNode);

                    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
                        for (auto &stat : insSeq->stats) {
                            classDef->rhs.emplace_back(std::move(stat));
                        }
                        classDef->rhs.emplace_back(std::move(insSeq->expr));
                    } else {
                        classDef->rhs.emplace_back(std::move(block->body));
                    }
                } else {
                    // We don't have a ClassMethods module yet. Let's create it and add the send definition to its
                    // members
                    auto loc = send->loc;
                    ast::ClassDef::RHS_store rhs;
                    auto *block = send->block();
                    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
                        for (auto &stat : insSeq->stats) {
                            rhs.emplace_back(std::move(stat));
                        }
                        rhs.emplace_back(std::move(insSeq->expr));
                    } else {
                        rhs.emplace_back(std::move(block->body));
                    }
                    auto name =
                        ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(), core::Names::Constants::ClassMethods());
                    classMethodsNode = ast::MK::Module(loc, loc, std::move(name), std::move(rhs));
                }
                continue;
            }
        } else if (auto mod = ast::cast_tree<ast::ClassDef>(stat)) {
            auto name = ast::cast_tree<ast::UnresolvedConstantLit>(mod->name);
            if (name && name->cnst == core::Names::Constants::ClassMethods()) {
                if (classMethodsNode) {
                    // ClassMethods module already exists. Let's add mod->rhs into existing module
                    auto classDef = ast::cast_tree<ast::ClassDef>(classMethodsNode);
                    for (auto &elem : mod->rhs) {
                        classDef->rhs.emplace_back(std::move(elem));
                    }
                } else {
                    // We don't have a ClassMethods module yet. Let's use the defined module (mod)
                    auto loc = mod->loc;
                    classMethodsNode = ast::MK::Module(loc, loc, std::move(mod->name), std::move(mod->rhs));
                }
                continue;
            }
        }
        // Everything else that we are not interested in should be copied as is
        stats.emplace_back(std::move(stat));
    }

    if (classMethodsNode) {
        // Generate a Send { Magic.mixes_in_class_methods(ClassMethods) }
        auto magic = ast::MK::Magic(klass->loc);
        auto classMethods =
            ast::MK::UnresolvedConstant(klass->loc, ast::MK::EmptyTree(), core::Names::Constants::ClassMethods());
        auto sendForMixes =
            ast::MK::Send2(klass->loc, std::move(magic), core::Names::mixesInClassMethods(),
                           klass->loc.copyWithZeroLength(), ast::MK::Self(klass->loc), std::move(classMethods));
        stats.emplace_back(std::move(sendForMixes));
    }

    klass->rhs.clear();
    klass->rhs.reserve(stats.size());
    if (classMethodsNode) {
        klass->rhs.emplace_back(std::move(classMethodsNode));
    }
    for (auto &stat : stats) {
        klass->rhs.emplace_back(std::move(stat));
    }
}

}; // namespace sorbet::rewriter
