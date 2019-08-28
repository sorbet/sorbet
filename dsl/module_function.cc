#include "dsl/module_function.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

vector<unique_ptr<ast::Expression>> ModuleFunction::replaceDSL(core::MutableContext ctx, ast::Send *send, 
                                                           const ast::Expression *prevStat) {
    vector<unique_ptr<ast::Expression>> stats;

    if (send->fun != core::Names::moduleFunction()) {
        return stats;
    }
    
    auto sig = ast::cast_tree_const<ast::Send>(prevStat);
    bool hasSig = sig && sig->fun == core::Names::sig();


    auto loc = send->loc;
    for (auto &arg : send->args) {
        ENFORCE(ast::isa_tree<ast::MethodDef>(arg.get()) || ast::isa_tree<ast::Literal>(arg.get()));
        if (auto defn = ast::cast_tree<ast::MethodDef>(arg.get())) {
            // this creates a private copy of the method
            unique_ptr<ast::Expression> privateCopy = defn->deepCopy();
            stats.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::private_(), move(privateCopy)));

            // as well as a public static copy of the method
            if (hasSig) {
                stats.emplace_back(sig->deepCopy());
            }
            unique_ptr<ast::Expression> moduleCopy = defn->deepCopy();
            ENFORCE(moduleCopy, "Should be non-nil.");
            auto newDefn = ast::cast_tree<ast::MethodDef>(moduleCopy.get());
            newDefn->flags |= ast::MethodDef::SelfMethod | ast::MethodDef::DSLSynthesized;
            stats.emplace_back(move(moduleCopy));
        } else if (auto name = ast::cast_tree<ast::Literal>(arg.get())) {

        }
    }

    return stats;
}
}
