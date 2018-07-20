#include "dsl/util.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet {
namespace dsl {

unique_ptr<ast::Expression> ASTUtil::dupType(ast::Expression *orig) {
    auto send = ast::cast_tree<ast::Send>(orig);
    if (send) {
        ast::Send::ARGS_store args;
        auto dupRecv = dupType(send->recv.get());
        if (!dupRecv) {
            return nullptr;
        }
        if (send->fun == core::Names::enum_()) {
            // T.enum() is weird, and accepts values instead of types. Just copy
            // it blindly through.
            return send->deepCopy();
        }
        for (auto &arg : send->args) {
            auto dupArg = dupType(arg.get());
            if (!dupArg) {
                // This isn't a Type signature, bail out
                return nullptr;
            }
            args.emplace_back(move(dupArg));
        }
        return ast::MK::Send(send->loc, move(dupRecv), send->fun, move(args));
    }

    auto ident = ast::cast_tree<ast::Ident>(orig);
    if (ident) {
        return ast::MK::Ident(ident->loc, ident->symbol);
    }

    auto cons = ast::cast_tree<ast::ConstantLit>(orig);
    if (!cons) {
        return nullptr;
    }

    auto scopeCnst = ast::cast_tree<ast::ConstantLit>(cons->scope.get());
    if (!scopeCnst) {
        if (ast::isa_tree<ast::EmptyTree>(cons->scope.get())) {
            return ast::MK::Constant(cons->loc, ast::MK::EmptyTree(cons->loc), cons->cnst);
        }
        auto *id = ast::cast_tree<ast::Ident>(cons->scope.get());
        ENFORCE(id != nullptr);
        ENFORCE(id->symbol == core::Symbols::root());
        return ast::MK::Constant(cons->loc, dupType(cons->scope.get()), cons->cnst);
    }
    auto scope = dupType(scopeCnst);
    if (scope == nullptr) {
        return nullptr;
    }
    return ast::MK::Constant(cons->loc, move(scope), cons->cnst);
}

unique_ptr<ast::Expression> ASTUtil::getHashValue(core::MutableContext ctx, ast::Hash *hash, core::NameRef name) {
    int i = -1;
    for (auto &keyExpr : hash->keys) {
        i++;
        auto *key = ast::cast_tree<ast::Literal>(keyExpr.get());
        if (key && key->isSymbol(ctx) && key->asSymbol(ctx) == name) {
            return move(hash->values[i]);
        }
    }
    return nullptr;
}

} // namespace dsl
} // namespace sorbet
