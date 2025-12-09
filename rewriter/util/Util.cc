#include "rewriter/util/Util.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"
#include "core/errors/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {
unique_ptr<ast::UnresolvedConstantLit> dupUnresolvedConstantLit(const ast::UnresolvedConstantLit *cons) {
    if (!cons) {
        return nullptr;
    }

    auto scopeCnst = ast::cast_tree<ast::UnresolvedConstantLit>(cons->scope);
    if (!scopeCnst) {
        if (ast::isa_tree<ast::EmptyTree>(cons->scope)) {
            return make_unique<ast::UnresolvedConstantLit>(cons->loc, ast::MK::EmptyTree(), cons->cnst);
        }
        auto id = ast::cast_tree<ast::ConstantLit>(cons->scope);
        if (id == nullptr) {
            return nullptr;
        }
        ENFORCE(id->symbol() == core::Symbols::root());
        return make_unique<ast::UnresolvedConstantLit>(cons->loc, ASTUtil::dupType(cons->scope), cons->cnst);
    }
    auto scope = ASTUtil::dupType(cons->scope);
    if (scope == nullptr) {
        return nullptr;
    }
    return make_unique<ast::UnresolvedConstantLit>(cons->loc, std::move(scope), cons->cnst);
}
} // namespace

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

            for (auto [key, value] : send->kwArgPairs()) {
                ENFORCE(ast::isa_tree<ast::Literal>(key));
                args.emplace_back(key.deepCopy());

                auto dupedValue = ASTUtil::dupType(value);
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

    auto ident = ast::cast_tree<ast::ConstantLit>(orig);
    if (ident) {
        auto orig = dupUnresolvedConstantLit(ident->original());
        if (ident->original() && !orig) {
            return nullptr;
        }
        if (orig == nullptr) {
            return ast::make_expression<ast::ConstantLit>(ident->loc(), ident->symbol());
        }

        return ast::make_expression<ast::ConstantLit>(ident->symbol(), std::move(orig));
    }

    auto arrayLit = ast::cast_tree<ast::Array>(orig);
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

    auto hashLit = ast::cast_tree<ast::Hash>(orig);
    if (hashLit != nullptr) {
        auto keys = ast::Hash::ENTRY_store{};
        auto values = ast::Hash::ENTRY_store{};
        ENFORCE(hashLit->keys.size() == hashLit->values.size());
        for (auto [key, value] : hashLit->kviter()) {
            auto keyLit = ast::cast_tree<ast::Literal>(key);
            if (keyLit == nullptr) {
                return nullptr;
            }

            auto duppedValue = dupType(value);
            if (duppedValue == nullptr) {
                return nullptr;
            }

            keys.emplace_back(key.deepCopy());
            values.emplace_back(std::move(duppedValue));
        }

        return ast::MK::Hash(hashLit->loc, std::move(keys), std::move(values));
    }

    return ast::ExpressionPtr::fromUnique(
        dupUnresolvedConstantLit(ast::cast_tree<ast::UnresolvedConstantLit>(orig).get()));
}

bool ASTUtil::hasHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name) {
    for (const auto &keyExpr : hash.keys) {
        auto key = ast::cast_tree<ast::Literal>(keyExpr);
        if (key && key->isSymbol() && key->asSymbol() == name) {
            return true;
        }
    }
    return false;
}

bool ASTUtil::hasTruthyHashValue(core::MutableContext ctx, const ast::Hash &hash, core::NameRef name) {
    for (auto [keyExpr, valExpr] : hash.kviter()) {
        auto key = ast::cast_tree<ast::Literal>(keyExpr);
        if (key && key->isSymbol() && key->asSymbol() == name) {
            auto val = ast::cast_tree<ast::Literal>(valExpr);
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
    for (auto [keyExpr, valExpr] : hash.kviter()) {
        auto key = ast::cast_tree<ast::Literal>(keyExpr);
        if (key && key->isSymbol() && key->asSymbol() == name) {
            auto key = std::move(keyExpr);
            auto value = std::move(valExpr);
            hash.keys.erase(&keyExpr);
            hash.values.erase(&valExpr);
            return make_pair(move(key), move(value));
        }
    }
    return make_pair(nullptr, nullptr);
}

namespace {

// This will return nullptr if the argument is not the right shape as a sig (i.e. a send to a method called `sig` with 0
// or 1 arguments, that in turn contains a block that contains a send) and it also checks the final method of the send
// against the provided `returns` (so that some uses can specifically look for `void` sigs while others can specifically
// look for non-void sigs).
template <typename T> T *castSigImpl(T *send) {
    static_assert(is_same_v<remove_const_t<T>, ast::Send>);

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
    auto body = ast::cast_tree<ast::Send>(block->body);
    while (body != nullptr && (body->fun == core::Names::checked() || body->fun == core::Names::onFailure() ||
                               body->fun == core::Names::override_() || body->fun == core::Names::overridable() ||
                               body->fun == core::Names::abstract())) {
        body = ast::cast_tree<ast::Send>(body->recv);
    }
    if (body != nullptr && (body->fun == core::Names::void_() || body->fun == core::Names::returns())) {
        return send;
    } else {
        return nullptr;
    }
}

} // namespace

ast::Send *ASTUtil::castSig(ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    if (send == nullptr) {
        return nullptr;
    }

    return castSigImpl(send.get());
}

const ast::Send *ASTUtil::castSig(const ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    if (send == nullptr) {
        return nullptr;
    }

    return castSigImpl(send.get());
}

ast::Send *ASTUtil::castSig(ast::Send *expr) {
    return castSigImpl(expr);
}

const ast::Send *ASTUtil::castSig(const ast::Send *expr) {
    return castSigImpl(expr);
}

ast::ExpressionPtr ASTUtil::mkKwArgsHash(const ast::Send *send) {
    if (!send->hasKwArgs() && !send->hasPosArgs()) {
        return nullptr;
    }

    ast::Hash::ENTRY_store keys;
    ast::Hash::ENTRY_store values;

    for (auto [key, value] : send->kwArgPairs()) {
        keys.emplace_back(key.deepCopy());
        values.emplace_back(value.deepCopy());
    }

    // handle a double-splat or a hash literal as the last argument
    bool explicitEmptyHash = false;
    if (send->hasKwSplat() || !send->hasKwArgs()) {
        if (auto hash = ast::cast_tree<ast::Hash>(send->hasKwSplat() ? *send->kwSplat()
                                                                     : send->getPosArg(send->numPosArgs() - 1))) {
            explicitEmptyHash = hash->keys.empty();
            for (auto [key, val] : hash->kviter()) {
                keys.emplace_back(key.deepCopy());
                values.emplace_back(val.deepCopy());
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
    return ast::MK::Method0(loc, loc, name, move(rhs), flags);
}

ast::ExpressionPtr ASTUtil::mkSet(core::Context ctx, core::LocOffsets loc, core::NameRef name, core::LocOffsets argLoc,
                                  ast::ExpressionPtr rhs, ast::MethodDef::Flags flags) {
    flags.isAttrBestEffortUIOnly = true;
    return ast::MK::Method1(loc, loc, name, ast::MK::Local(argLoc, core::Names::arg0()), move(rhs), flags);
}

ast::ExpressionPtr ASTUtil::mkSyntheticGet(core::Context ctx, core::LocOffsets loc, core::NameRef name,
                                           ast::ExpressionPtr rhs, ast::MethodDef::Flags flags) {
    flags.isAttrBestEffortUIOnly = true;
    return ast::MK::SyntheticMethod0(loc, loc, name, move(rhs), flags);
}

ast::ExpressionPtr ASTUtil::mkSyntheticSet(core::Context ctx, core::LocOffsets loc, core::NameRef name,
                                           core::LocOffsets argLoc, ast::ExpressionPtr rhs,
                                           ast::MethodDef::Flags flags) {
    flags.isAttrBestEffortUIOnly = true;
    return ast::MK::SyntheticMethod1(loc, loc, name, ast::MK::Local(argLoc, core::Names::arg0()), move(rhs), flags);
}

ast::ExpressionPtr ASTUtil::mkNilable(core::LocOffsets loc, ast::ExpressionPtr type) {
    return ast::MK::Send1(loc, ast::MK::T(loc), core::Names::nilable(), loc.copyWithZeroLength(), move(type));
}

ast::ExpressionPtr ASTUtil::thunkBody(core::MutableContext ctx, ast::ExpressionPtr &node) {
    auto send = ast::cast_tree<ast::Send>(node);
    if (send == nullptr) {
        return nullptr;
    }
    if (send->fun != core::Names::lambda() && send->fun != core::Names::proc()) {
        return nullptr;
    }
    // Valid receivers for lambda/proc are either a self reference or `Kernel`
    if (!send->recv.isSelfReference() && !ast::MK::isKernelApproximate(send->recv)) {
        return nullptr;
    }
    if (!send->hasBlock()) {
        return nullptr;
    }
    auto *block = send->block();
    if (!block->params.empty()) {
        return nullptr;
    }
    return std::move(block->body);
}

namespace {

bool validAttrName(core::MutableContext ctx, core::LocOffsets loc, core::NameRef name) {
    auto shortName = name.shortName(ctx);

    auto validName = !shortName.empty() && (isalpha(shortName.front()) || shortName.front() == '_') &&
                     absl::c_all_of(shortName, [](char c) { return isalnum(c) || c == '_'; });

    if (!validName) {
        if (auto e = ctx.beginIndexerError(loc, core::errors::Rewriter::BadAttrArg)) {
            if (shortName.empty()) {
                e.setHeader("Attribute names must be non-empty");
            } else {
                e.setHeader("Bad attribute name `{}`", shortName);
            }
        }
    }

    return validName;
}

} // namespace

pair<core::NameRef, core::LocOffsets> ASTUtil::getAttrName(core::MutableContext ctx, core::NameRef attrFun,
                                                           const ast::ExpressionPtr &name) {
    core::LocOffsets loc;
    core::NameRef res;
    if (auto lit = ast::cast_tree<ast::Literal>(name)) {
        if (lit->isSymbol()) {
            res = lit->asSymbol();
            loc = lit->loc;
            if (!validAttrName(ctx, loc, res)) {
                return make_pair(core::NameRef::noName(), lit->loc);
            }

            ENFORCE(ctx.locAt(loc).exists());
            ENFORCE(ctx.locAt(loc).source(ctx).value().size() > 1 && ctx.locAt(loc).source(ctx).value()[0] == ':');
            loc = core::LocOffsets{loc.beginPos() + 1, loc.endPos()};
        } else if (lit->isString()) {
            res = lit->asString();
            loc = lit->loc;
            if (!validAttrName(ctx, loc, res)) {
                return make_pair(core::NameRef::noName(), lit->loc);
            }

            DEBUG_ONLY({
                auto l = ctx.locAt(loc);
                ENFORCE(l.exists());
                auto source = l.source(ctx).value();
                ENFORCE(source.size() > 2);
                auto firstChar = source[0];
                ENFORCE(firstChar == '"' || firstChar == '\'');
                auto lastChar = source[source.size() - 1];
                ENFORCE(lastChar == firstChar);
            });
            loc = core::LocOffsets{loc.beginPos() + 1, loc.endPos() - 1};
        }
    }
    if (!res.exists()) {
        if (auto e = ctx.beginIndexerError(name.loc(), core::errors::Rewriter::BadAttrArg)) {
            e.setHeader("Argument to `{}` must be a Symbol or String", attrFun.shortName(ctx));
        }
    }
    return make_pair(res, loc);
}

bool ASTUtil::isRootScopedSyntacticConstant(const ast::ExpressionPtr &expr,
                                            absl::Span<const core::NameRef> constantName) {
    auto *p = &expr;

    for (auto it = constantName.rbegin(), end = constantName.rend(); it != end; ++it) {
        auto ucl = ast::cast_tree<ast::UnresolvedConstantLit>(*p);

        if (ucl == nullptr || ucl->cnst != *it) {
            return false;
        }

        p = &ucl->scope;
    }

    return ast::MK::isRootScope(*p);
}

optional<ASTUtil::DuplicateArg> ASTUtil::findDuplicateArg(core::MutableContext ctx, const ast::Send *send) {
    if (!send) {
        return nullopt;
    }

    UnorderedMap<core::NameRef, core::LocOffsets> seenNames;

    for (auto &arg : send->posArgs()) {
        auto lit = ast::cast_tree<ast::Literal>(arg);
        if (!lit || !lit->isName()) {
            continue;
        }

        auto name = lit->asName();
        auto loc = lit->loc;

        auto [it, inserted] = seenNames.emplace(name, loc);
        if (!inserted) {
            return DuplicateArg{name, it->second, loc};
        }
    }

    return nullopt;
}

} // namespace sorbet::rewriter
