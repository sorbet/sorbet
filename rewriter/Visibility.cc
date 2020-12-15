#include "rewriter/Visibility.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

void Visibility::run(core::MutableContext ctx, ast::ClassDef *classDef) {
    core::NameRef currentVisibility = core::Names::public_();
    vector<ast::TreePtr> inlinePrivateCalls;

    for (auto &stat : classDef->rhs) {
        cout << "iterating over stat nodes\n";
        if (auto *send = ast::cast_tree<ast::Send>(stat)) {
            cout << "START PRINT STATEMENT\n\n";
            cout << "isSelfReference:" << send->recv.isSelfReference() << "\n";
            cout << "\n================================\n\n";
            if ((send->recv == nullptr || send->recv.isSelfReference()) && send->args.size() == 0) {
                cout << "in initial if statement";
                // Technically this node can now be removed, but it doesn't really need to be removed now
                if (send->fun == core::Names::private_()) {
                    currentVisibility = core::Names::private_();
                } else if (send->fun == core::Names::protected_()) {
                    currentVisibility = core::Names::protected_();
                } else if (send->fun == core::Names::public_()) {
                    currentVisibility = core::Names::public_();
                }
            }
        } else if (auto *methodDef = ast::cast_tree<ast::MethodDef>(stat)) {
            if (currentVisibility == core::Names::private_()) {
                auto privateCall =
                    ast::MK::Send1(methodDef->declLoc, ast::MK::Self(methodDef->declLoc), core::Names::private_(),
                                   ast::MK::Symbol(methodDef->declLoc, methodDef->name));
                inlinePrivateCalls.emplace_back(std::move(privateCall));
            }
        }
    }

    for (auto &newPrivateCall : inlinePrivateCalls) {
        classDef->rhs.emplace_back(std::move(newPrivateCall));
    }
}

}; // namespace sorbet::rewriter
