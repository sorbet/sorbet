#include "dsl/util.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

unique_ptr<ast::Expression> ASTUtil::dupType(const ast::Expression *orig) {
    auto send = ast::cast_tree_const<ast::Send>(orig);
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
            args.emplace_back(std::move(dupArg));
        }
        return ast::MK::Send(send->loc, std::move(dupRecv), send->fun, std::move(args));
    }

    auto ident = ast::cast_tree_const<ast::ConstantLit>(orig);
    if (ident) {
        auto orig = dupType(ident->original.get());
        if (ident->original && !orig) {
            return nullptr;
        }
        auto ptr = ast::cast_tree<ast::UnresolvedConstantLit>(orig.get());
        orig.release();
        return make_unique<ast::ConstantLit>(ident->loc, ident->typeAliasOrConstantSymbol(),
                                             unique_ptr<ast::UnresolvedConstantLit>(ptr), ident->typeAlias);
    }

    auto cons = ast::cast_tree_const<ast::UnresolvedConstantLit>(orig);
    if (!cons) {
        return nullptr;
    }

    auto scopeCnst = ast::cast_tree_const<ast::UnresolvedConstantLit>(cons->scope.get());
    if (!scopeCnst) {
        if (ast::isa_tree<ast::EmptyTree>(cons->scope.get())) {
            return ast::MK::UnresolvedConstant(cons->loc, ast::MK::EmptyTree(), cons->cnst);
        }
        auto *id = ast::cast_tree_const<ast::ConstantLit>(cons->scope.get());
        if (id == nullptr) {
            return nullptr;
        }
        ENFORCE(id->constantSymbol() == core::Symbols::root());
        return ast::MK::UnresolvedConstant(cons->loc, dupType(cons->scope.get()), cons->cnst);
    }
    auto scope = dupType(scopeCnst);
    if (scope == nullptr) {
        return nullptr;
    }
    return ast::MK::UnresolvedConstant(cons->loc, std::move(scope), cons->cnst);
}

bool ASTUtil::hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name) {
    int i = -1;
    for (const auto &keyExpr : hash.keys) {
        i++;
        auto *key = ast::cast_tree_const<ast::Literal>(keyExpr.get());
        if (key && key->isSymbol(ctx) && key->asSymbol(ctx) == name) {
            auto val = ast::cast_tree_const<ast::Literal>(hash.values[i].get());
            if (!val) {
                // All non-literals are truthy
                return true;
            }
            if (val->isNil(ctx) || val->isFalse(ctx)) {
                return false;
            }
            return true;
        }
    }
    return false;
}

pair<unique_ptr<ast::Expression>, unique_ptr<ast::Expression>>
ASTUtil::extractHashValue(core::MutableContext ctx, ast::Hash &hash, core::NameRef name) {
    int i = -1;
    for (auto &keyExpr : hash.keys) {
        i++;
        auto *key = ast::cast_tree<ast::Literal>(keyExpr.get());
        if (key && key->isSymbol(ctx) && key->asSymbol(ctx) == name) {
            auto key = std::move(keyExpr);
            auto value = std::move(hash.values[i]);
            hash.keys.erase(hash.keys.begin() + i);
            hash.values.erase(hash.values.begin() + i);
            return make_pair(move(key), move(value));
        }
    }
    return make_pair(nullptr, nullptr);
}

void ASTUtil::putBackHashValue(core::MutableContext ctx, ast::Hash &hash, unique_ptr<ast::Expression> key,
                               unique_ptr<ast::Expression> value) {
    hash.keys.emplace_back(move(key));
    hash.values.emplace_back(move(value));
}

} // namespace sorbet::dsl
