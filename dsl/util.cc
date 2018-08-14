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

    auto ident = ast::cast_tree<ast::ConstantLit>(orig);
    if (ident) {
        auto orig = dupType(ident->original.get());
        if (ident->original && !orig) {
            return nullptr;
        }
        auto ptr = ast::cast_tree<ast::UnresolvedConstantLit>(orig.get());
        orig.release();
        auto typeAlias = dupType(ident->typeAlias.get());
        if (ident->typeAlias && !typeAlias) {
            return nullptr;
        }
        return make_unique<ast::ConstantLit>(ident->loc, ident->symbol, unique_ptr<ast::UnresolvedConstantLit>(ptr),
                                             move(typeAlias));
    }

    auto cons = ast::cast_tree<ast::UnresolvedConstantLit>(orig);
    if (!cons) {
        return nullptr;
    }

    auto scopeCnst = ast::cast_tree<ast::UnresolvedConstantLit>(cons->scope.get());
    if (!scopeCnst) {
        if (ast::isa_tree<ast::EmptyTree>(cons->scope.get())) {
            return ast::MK::UnresolvedConstant(cons->loc, ast::MK::EmptyTree(cons->loc), cons->cnst);
        }
        auto *id = ast::cast_tree<ast::ConstantLit>(cons->scope.get());
        ENFORCE(id != nullptr);
        ENFORCE(id->symbol == core::Symbols::root());
        return ast::MK::UnresolvedConstant(cons->loc, dupType(cons->scope.get()), cons->cnst);
    }
    auto scope = dupType(scopeCnst);
    if (scope == nullptr) {
        return nullptr;
    }
    return ast::MK::UnresolvedConstant(cons->loc, move(scope), cons->cnst);
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
