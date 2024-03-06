#include "rewriter/Util.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

ast::ExpressionPtr ASTUtil::dupType(const ast::ExpressionPtr &orig) {
    auto send = ast::cast_tree<ast::Send>(orig);
    if (send) {
        auto dupRecv = dupType(send->recv);
        if (!dupRecv) {
            return nullptr;
        }
        if (send->fun == core::Names::enum_() || send->fun == core::Names::deprecatedEnum()) {
            // T.deprecated_enum() is weird, and accepts values instead of types. Just copy
            // it blindly through.
            return send->deepCopy();
        }

        if (send->fun == core::Names::params() && !send->hasPosArgs() && !send->hasKwSplat()) {
            // T.proc.params takes inlined keyword argument pairs, and can't handle kwsplat
            ast::Send::ARGS_store args;

            const auto numKwArgs = send->numKwArgs();
            for (auto i = 0; i < numKwArgs; ++i) {
                ENFORCE(ast::isa_tree<ast::Literal>(send->getKwKey(i)));
                args.emplace_back(send->getKwKey(i).deepCopy());

                auto dupedValue = ASTUtil::dupType(send->getKwValue(i));
                if (dupedValue == nullptr) {
                    return nullptr;
                }

                args.emplace_back(std::move(dupedValue));
            }

            return ast::MK::Send(send->loc, std::move(dupRecv), send->fun, send->funLoc, 0, std::move(args));
        }

        ast::Send::ARGS_store args;
        for (auto &arg : send->nonBlockArgs()) {
            auto dupArg = dupType(arg);
            if (!dupArg) {
                // This isn't a Type signature, bail out
                return nullptr;
            }
            args.emplace_back(std::move(dupArg));
        }

        return ast::MK::Send(send->loc, std::move(dupRecv), send->fun, send->funLoc, send->numPosArgs(),
                             std::move(args));
    }

    auto *ident = ast::cast_tree<ast::ConstantLit>(orig);
    if (ident) {
        auto orig = dupType(ident->original);
        if (ident->original && !orig) {
            return nullptr;
        }
        return ast::make_expression<ast::ConstantLit>(ident->loc, ident->symbol, std::move(orig));
    }

    auto *arrayLit = ast::cast_tree<ast::Array>(orig);
    if (arrayLit != nullptr) {
        auto elems = ast::Array::ENTRY_store{};
        for (const auto &elem : arrayLit->elems) {
            auto duppedElem = dupType(elem);
            if (duppedElem == nullptr) {
                return nullptr;
            }

            elems.emplace_back(std::move(duppedElem));
        }

        return ast::MK::Array(arrayLit->loc, std::move(elems));
    }

    auto *hashLit = ast::cast_tree<ast::Hash>(orig);
    if (hashLit != nullptr) {
        auto keys = ast::Hash::ENTRY_store{};
        auto values = ast::Hash::ENTRY_store{};
        ENFORCE(hashLit->keys.size() == hashLit->values.size());
        for (size_t i = 0; i < hashLit->keys.size(); i++) {
            const auto &key = hashLit->keys[i];

            auto *keyLit = ast::cast_tree<ast::Literal>(key);
            if (keyLit == nullptr) {
                return nullptr;
            }

            const auto &value = hashLit->values[i];
            auto duppedValue = dupType(value);
            if (duppedValue == nullptr) {
                return nullptr;
            }

            keys.emplace_back(key.deepCopy());
            values.emplace_back(std::move(duppedValue));
        }

        return ast::MK::Hash(hashLit->loc, std::move(keys), std::move(values));
    }

    auto *cons = ast::cast_tree<ast::UnresolvedConstantLit>(orig);
    if (!cons) {
        return nullptr;
    }

    auto *scopeCnst = ast::cast_tree<ast::UnresolvedConstantLit>(cons->scope);
    if (!scopeCnst) {
        if (ast::isa_tree<ast::EmptyTree>(cons->scope)) {
            return ast::MK::UnresolvedConstant(cons->loc, ast::MK::EmptyTree(), cons->cnst);
        }
        auto *id = ast::cast_tree<ast::ConstantLit>(cons->scope);
        if (id == nullptr) {
            return nullptr;
        }
        ENFORCE(id->symbol == core::Symbols::root());
        return ast::MK::UnresolvedConstant(cons->loc, dupType(cons->scope), cons->cnst);
    }
    auto scope = dupType(cons->scope);
    if (scope == nullptr) {
        return nullptr;
    }
    return ast::MK::UnresolvedConstant(cons->loc, std::move(scope), cons->cnst);
}

bool ASTUtil::hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name) {
    for (const auto &keyExpr : hash.keys) {
        auto *key = ast::cast_tree<ast::Literal>(keyExpr);
        if (key && key->isSymbol() && key->asSymbol() == name) {
            return true;
        }
    }
    return false;
}

bool ASTUtil::hasTruthyHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name) {
    int i = -1;
    for (const auto &keyExpr : hash.keys) {
        i++;
        auto *key = ast::cast_tree<ast::Literal>(keyExpr);
        if (key && key->isSymbol() && key->asSymbol() == name) {
            auto *val = ast::cast_tree<ast::Literal>(hash.values[i]);
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

pair<ast::ExpressionPtr, ast::ExpressionPtr> ASTUtil::extractHashValue(core::MutableContext ctx, ast::Hash &hash,
                                                                       core::NameRef name) {
    int i = -1;
    for (auto &keyExpr : hash.keys) {
        i++;
        auto *key = ast::cast_tree<ast::Literal>(keyExpr);
        if (key && key->isSymbol() && key->asSymbol() == name) {
            auto key = std::move(keyExpr);
            auto value = std::move(hash.values[i]);
            hash.keys.erase(hash.keys.begin() + i);
            hash.values.erase(hash.values.begin() + i);
            return make_pair(move(key), move(value));
        }
    }
    return make_pair(nullptr, nullptr);
}

ast::Send *ASTUtil::castSig(ast::ExpressionPtr &expr) {
    auto *send = ast::cast_tree<ast::Send>(expr);
    if (send == nullptr) {
        return nullptr;
    }

    return ASTUtil::castSig(send);
}

// This will return nullptr if the argument is not the right shape as a sig (i.e. a send to a method called `sig` with 0
// or 1 arguments, that in turn contains a block that contains a send) and it also checks the final method of the send
// against the provided `returns` (so that some uses can specifically look for `void` sigs while others can specifically
// look for non-void sigs).
ast::Send *ASTUtil::castSig(ast::Send *send) {
    if (send->fun != core::Names::sig()) {
        return nullptr;
    }
    if (!send->hasBlock()) {
        return nullptr;
    }
    // 0 args is common case
    // 1 arg  is `sig(:final)`
    // 2 args is `Sorbet::Private::Static.sig(self, :final)`
    if (send->numPosArgs() > 2) {
        return nullptr;
    }
    auto *block = send->block();
    ENFORCE(block);
    auto *body = ast::cast_tree<ast::Send>(block->body);
    while (body != nullptr && (body->fun == core::Names::checked() || body->fun == core::Names::onFailure())) {
        body = ast::cast_tree<ast::Send>(body->recv);
    }
    if (body != nullptr && (body->fun == core::Names::void_() || body->fun == core::Names::returns())) {
        return send;
    } else {
        return nullptr;
    }
}

ast::ExpressionPtr ASTUtil::mkKwArgsHash(const ast::Send *send) {
    if (!send->hasKwArgs() && !send->hasPosArgs()) {
        return nullptr;
    }

    ast::Hash::ENTRY_store keys;
    ast::Hash::ENTRY_store values;

    const auto numKwArgs = send->numKwArgs();
    for (auto i = 0; i < numKwArgs; ++i) {
        keys.emplace_back(send->getKwKey(i).deepCopy());
        values.emplace_back(send->getKwValue(i).deepCopy());
    }

    // handle a double-splat or a hash literal as the last argument
    bool explicitEmptyHash = false;
    if (send->hasKwSplat() || !send->hasKwArgs()) {
        if (auto *hash = ast::cast_tree<ast::Hash>(send->hasKwSplat() ? *send->kwSplat()
                                                                      : send->getPosArg(send->numPosArgs() - 1))) {
            explicitEmptyHash = hash->keys.empty();
            for (auto i = 0; i < hash->keys.size(); ++i) {
                keys.emplace_back(hash->keys[i].deepCopy());
                values.emplace_back(hash->values[i].deepCopy());
            }
        }
    }

    if (!keys.empty() || explicitEmptyHash) {
        return ast::MK::Hash(send->loc, std::move(keys), std::move(values));
    } else {
        return nullptr;
    }
}

ast::ExpressionPtr ASTUtil::mkGet(core::Context ctx, core::LocOffsets loc, core::NameRef name, ast::ExpressionPtr rhs,
                                  ast::MethodDef::Flags flags) {
    flags.isAttrBestEffortUIOnly = true;
    auto ret = ast::MK::SyntheticMethod0(loc, loc, name, move(rhs), flags);
    return ret;
}

ast::ExpressionPtr ASTUtil::mkSet(core::Context ctx, core::LocOffsets loc, core::NameRef name, core::LocOffsets argLoc,
                                  ast::ExpressionPtr rhs, ast::MethodDef::Flags flags) {
    flags.isAttrBestEffortUIOnly = true;
    return ast::MK::SyntheticMethod1(loc, loc, name, ast::MK::ResolvedLocal(argLoc, core::Names::arg0()), move(rhs), flags);
}

ast::ExpressionPtr ASTUtil::mkNilable(core::LocOffsets loc, ast::ExpressionPtr type) {
    return ast::MK::Send1(loc, ast::MK::T(loc), core::Names::nilable(), loc.copyWithZeroLength(), move(type));
}

namespace {

// Returns `true` when the expression passed is an UnresolvedConstantLit with the name `Kernel` and no additional scope.
bool isKernel(const ast::ExpressionPtr &expr) {
    if (auto *constRecv = ast::cast_tree<ast::UnresolvedConstantLit>(expr)) {
        return ast::isa_tree<ast::EmptyTree>(constRecv->scope) && constRecv->cnst == core::Names::Constants::Kernel();
    }
    return false;
}

} // namespace

ast::ExpressionPtr ASTUtil::thunkBody(core::MutableContext ctx, ast::ExpressionPtr &node) {
    auto *send = ast::cast_tree<ast::Send>(node);
    if (send == nullptr) {
        return nullptr;
    }
    if (send->fun != core::Names::lambda() && send->fun != core::Names::proc()) {
        return nullptr;
    }
    // Valid receivers for lambda/proc are either a self reference or `Kernel`
    if (!send->recv.isSelfReference() && !isKernel(send->recv)) {
        return nullptr;
    }
    if (!send->hasBlock()) {
        return nullptr;
    }
    auto *block = send->block();
    if (!block->args.empty()) {
        return nullptr;
    }
    return std::move(block->body);
}

} // namespace sorbet::rewriter
