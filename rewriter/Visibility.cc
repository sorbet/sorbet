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

    for (auto &stat : classDef->rhs) {
        typecase(
            stat.get(),
            [&](ast::Send *send) {
                if (send->args.size() != 0 && send->recv != nullptr) {
                    return;
                }
                // Technically this node can now be removed, but it doesn't really need to be removed now
                if (send->fun == core::Names::private_()) {
                    currentVisibility = core::Names::private_();
                } else if (send->fun == core::Names::protected_()) {
                    currentVisibility = core::Names::protected_();
                } else if (send->fun == core::Names::public_()) {
                    currentVisibility = core::Names::public_();
                }
            },
            [&](ast::MethodDef *mdef) {
                if (currentVisibility == core::Names::private_()) {
                    auto privateCall = ast::MK::Send1(mdef->loc, ast::MK::Self(mdef->loc), core::Names::private_(),
                                                      ast::MK::Symbol(mdef->loc, mdef->name));
                    classDef->rhs.insert(classDef->rhs.end(), std::move(privateCall));
                }
            },
            [&](ast::Expression *e) {});
    }
}

}; // namespace sorbet::rewriter
