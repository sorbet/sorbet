#include "ast/ast.h"
#include <type_traits>

using namespace std;

namespace sorbet::ast {

namespace {

bool deepEqual(const void *avoid, const ExpressionPtr &tree, const ExpressionPtr &other, bool root = false);

template <unsigned long N>
bool deepEqualVec(const void *avoid, const InlinedVector<ExpressionPtr, N> &a,
                  const InlinedVector<ExpressionPtr, N> &b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (int i = 0; i < a.size(); i++) {
        if (!deepEqual(avoid, a[i], b[i])) {
            return false;
        }
    }
    return true;
}

class DeepEqualError {};

bool deepEqual(const void *avoid, const Tag tag, const void *tree, const void *other, bool root) {
    if (!root && tree == avoid) {
        throw DeepEqualError();
    }

    switch (tag) {
        case Tag::EmptyTree:
            return true;

        case Tag::Send: {
            auto *a = reinterpret_cast<const Send *>(tree);
            auto *b = reinterpret_cast<const Send *>(other);
            return deepEqual(avoid, a->recv, b->recv) && a->fun == b->fun &&
                   deepEqualVec(avoid, a->rawArgsDoNotUse(), b->rawArgsDoNotUse());
        }

        case Tag::ClassDef: {
            auto *a = reinterpret_cast<const ClassDef *>(tree);
            auto *b = reinterpret_cast<const ClassDef *>(other);
            if (a->symbol != b->symbol) {
                return false;
            }
            if (!deepEqual(avoid, a->name, b->name)) {
                return false;
            }
            if (!deepEqualVec(avoid, a->ancestors, b->ancestors)) {
                return false;
            }
            if (!deepEqualVec(avoid, a->singletonAncestors, b->singletonAncestors)) {
                return false;
            }
            if (!deepEqualVec(avoid, a->rhs, b->rhs)) {
                return false;
            }
            if (a->kind != b->kind) {
                return false;
            }
            return true;
        }

        case Tag::MethodDef: {
            auto *a = reinterpret_cast<const MethodDef *>(tree);
            auto *b = reinterpret_cast<const MethodDef *>(other);
            if (a->symbol != b->symbol) {
                return false;
            }
            if (a->name != b->name) {
                return false;
            }
            if (!deepEqualVec(avoid, a->args, b->args)) {
                return false;
            }
            if (!deepEqual(avoid, a->rhs, b->rhs)) {
                return false;
            }
            if (a->flags != b->flags) {
                return false;
            }
            return true;
        }

        case Tag::If: {
            auto *a = reinterpret_cast<const If *>(tree);
            auto *b = reinterpret_cast<const If *>(other);
            return deepEqual(avoid, a->cond, b->cond) && deepEqual(avoid, a->thenp, b->thenp) &&
                   deepEqual(avoid, a->elsep, b->elsep);
        }

        case Tag::While: {
            auto *a = reinterpret_cast<const While *>(tree);
            auto *b = reinterpret_cast<const While *>(other);
            return deepEqual(avoid, a->cond, b->cond) && deepEqual(avoid, a->body, b->body);
        }

        case Tag::Break: {
            auto *a = reinterpret_cast<const Break *>(tree);
            auto *b = reinterpret_cast<const Break *>(other);
            if (!deepEqual(avoid, a->expr, b->expr)) {
                return false;
            }
            return true;
        }

        case Tag::Retry: {
            return true;
        }

        case Tag::Next: {
            auto *a = reinterpret_cast<const Next *>(tree);
            auto *b = reinterpret_cast<const Next *>(other);
            return deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::Return: {
            auto *a = reinterpret_cast<const Return *>(tree);
            auto *b = reinterpret_cast<const Return *>(other);
            return deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::RescueCase: {
            auto *a = reinterpret_cast<const RescueCase *>(tree);
            auto *b = reinterpret_cast<const RescueCase *>(other);
            return deepEqualVec(avoid, a->exceptions, b->exceptions) && deepEqual(avoid, a->var, b->var) &&
                   deepEqual(avoid, a->body, b->body);
        }

        case Tag::Rescue: {
            auto *a = reinterpret_cast<const Rescue *>(tree);
            auto *b = reinterpret_cast<const Rescue *>(other);
            return deepEqual(avoid, a->body, b->body) && deepEqualVec(avoid, a->rescueCases, b->rescueCases) &&
                   deepEqual(avoid, a->else_, b->else_) && deepEqual(avoid, a->ensure, b->ensure);
        }

        case Tag::Local: {
            auto *a = reinterpret_cast<const Local *>(tree);
            auto *b = reinterpret_cast<const Local *>(other);
            return a->localVariable == b->localVariable;
        }

        case Tag::UnresolvedIdent: {
            auto *a = reinterpret_cast<const UnresolvedIdent *>(tree);
            auto *b = reinterpret_cast<const UnresolvedIdent *>(other);
            return a->kind == b->kind && a->name == b->name;
        }

        case Tag::RestArg: {
            auto *a = reinterpret_cast<const RestArg *>(tree);
            auto *b = reinterpret_cast<const RestArg *>(other);
            return deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::KeywordArg: {
            auto *a = reinterpret_cast<const KeywordArg *>(tree);
            auto *b = reinterpret_cast<const KeywordArg *>(other);
            return deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::OptionalArg: {
            auto *a = reinterpret_cast<const OptionalArg *>(tree);
            auto *b = reinterpret_cast<const OptionalArg *>(other);
            return deepEqual(avoid, a->expr, b->expr) && deepEqual(avoid, a->default_, b->default_);
        }

        case Tag::BlockArg: {
            auto *a = reinterpret_cast<const BlockArg *>(tree);
            auto *b = reinterpret_cast<const BlockArg *>(other);
            return deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::ShadowArg: {
            auto *a = reinterpret_cast<const ShadowArg *>(tree);
            auto *b = reinterpret_cast<const ShadowArg *>(other);
            return deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::Assign: {
            auto *a = reinterpret_cast<const Assign *>(tree);
            auto *b = reinterpret_cast<const Assign *>(other);
            return deepEqual(avoid, a->lhs, b->lhs) && deepEqual(avoid, a->rhs, b->rhs);
        }

        case Tag::Cast: {
            auto *a = reinterpret_cast<const Cast *>(tree);
            auto *b = reinterpret_cast<const Cast *>(other);
            return a->type == b->type && deepEqual(avoid, a->arg, b->arg) && a->cast == b->cast &&
                   deepEqual(avoid, a->typeExpr, b->typeExpr);
        }

        case Tag::Hash: {
            auto *a = reinterpret_cast<const Hash *>(tree);
            auto *b = reinterpret_cast<const Hash *>(other);
            return deepEqualVec(avoid, a->keys, b->keys) && deepEqualVec(avoid, a->values, b->values);
        }
        case Tag::Array: {
            auto *a = reinterpret_cast<const Array *>(tree);
            auto *b = reinterpret_cast<const Array *>(other);
            return deepEqualVec(avoid, a->elems, b->elems);
        }

        case Tag::Literal: {
            auto *a = reinterpret_cast<const Literal *>(tree);
            auto *b = reinterpret_cast<const Literal *>(other);
            auto aType = a->value;
            auto bType = b->value;
            if (aType.tag() != bType.tag()) {
                return false;
            }
            if (aType.tag() == core::TypePtr::Tag::IntegerLiteralType) {
                return core::cast_type_nonnull<core::IntegerLiteralType>(aType).value ==
                       core::cast_type_nonnull<core::IntegerLiteralType>(bType).value;
            } else if (aType.tag() == core::TypePtr::Tag::FloatLiteralType) {
                return core::cast_type_nonnull<core::FloatLiteralType>(aType).value ==
                       core::cast_type_nonnull<core::FloatLiteralType>(bType).value;
            } else if (aType.tag() == core::TypePtr::Tag::NamedLiteralType) {
                auto named_literal_a = core::cast_type_nonnull<core::NamedLiteralType>(aType);
                auto named_literal_b = core::cast_type_nonnull<core::NamedLiteralType>(bType);
                return named_literal_a.literalKind == named_literal_b.literalKind &&
                       named_literal_a.asName() == named_literal_b.asName();
            } else if (aType.tag() == core::TypePtr::Tag::ClassType) {
                auto class_type_a = core::cast_type_nonnull<core::ClassType>(aType);
                auto class_type_b = core::cast_type_nonnull<core::ClassType>(bType);
                return class_type_a.symbol == class_type_b.symbol;
            } else {
                // TODO(neil): are there any more cases we need to handle?
                ENFORCE(false, "unexpected TypePtr::Tag: {}", aType.tag());
            }
            return false;
        }

        case Tag::UnresolvedConstantLit: {
            auto *a = reinterpret_cast<const UnresolvedConstantLit *>(tree);
            auto *b = reinterpret_cast<const UnresolvedConstantLit *>(other);
            return deepEqual(avoid, a->scope, b->scope) && a->cnst == b->cnst;
        }

        case Tag::ConstantLit: {
            auto *a = reinterpret_cast<const ConstantLit *>(tree);
            auto *b = reinterpret_cast<const ConstantLit *>(other);
            return a->symbol == b->symbol && deepEqual(avoid, a->original, b->original);
        }

        case Tag::ZSuperArgs: {
            return true;
        }

        case Tag::Block: {
            auto *a = reinterpret_cast<const Block *>(tree);
            auto *b = reinterpret_cast<const Block *>(other);
            return deepEqualVec(avoid, a->args, b->args) && deepEqual(avoid, a->body, b->body);
        }

        case Tag::InsSeq: {
            auto *a = reinterpret_cast<const InsSeq *>(tree);
            auto *b = reinterpret_cast<const InsSeq *>(other);
            return deepEqualVec(avoid, a->stats, b->stats) && deepEqual(avoid, a->expr, b->expr);
        }

        case Tag::RuntimeMethodDefinition: {
            auto *a = reinterpret_cast<const RuntimeMethodDefinition *>(tree);
            auto *b = reinterpret_cast<const RuntimeMethodDefinition *>(other);
            return a->name == b->name && a->isSelfMethod == b->isSelfMethod;
        }
    }
}

bool deepEqual(const void *avoid, const ExpressionPtr &tree, const ExpressionPtr &other, bool root) {
    ENFORCE(tree != nullptr);
    ENFORCE(tree != nullptr);
    if (tree.tag() != other.tag()) {
        return false;
    }

    return deepEqual(avoid, tree.tag(), tree.get(), other.get(), root);
}

} // namespace

bool ExpressionPtr::deepEqual(const ExpressionPtr &other) const {
    if (tag() != other.tag()) {
        return false;
    }
    try {
        return sorbet::ast::deepEqual(get(), tag(), get(), other.get(), true);
    } catch (DeepEqualError &e) {
        return false;
    }
}

#define EQUAL_IMPL(name)                                                                                \
    bool name::deepEqual(const ExpressionPtr &other) const {                                            \
        if (ExpressionToTag<name>::value != other.tag()) {                                              \
            return false;                                                                               \
        }                                                                                               \
        try {                                                                                           \
            return sorbet::ast::deepEqual(this, ExpressionToTag<name>::value, this, other.get(), true); \
        } catch (DeepEqualError & e) {                                                                  \
            return false;                                                                               \
        }                                                                                               \
    }

EQUAL_IMPL(EmptyTree);
EQUAL_IMPL(Send);
EQUAL_IMPL(ClassDef);
EQUAL_IMPL(MethodDef);
EQUAL_IMPL(If);
EQUAL_IMPL(While);
EQUAL_IMPL(Break);
EQUAL_IMPL(Retry);
EQUAL_IMPL(Next);
EQUAL_IMPL(Return);
EQUAL_IMPL(RescueCase);
EQUAL_IMPL(Rescue);
EQUAL_IMPL(Local);
EQUAL_IMPL(UnresolvedIdent);
EQUAL_IMPL(RestArg);
EQUAL_IMPL(KeywordArg);
EQUAL_IMPL(OptionalArg);
EQUAL_IMPL(BlockArg);
EQUAL_IMPL(ShadowArg);
EQUAL_IMPL(Assign);
EQUAL_IMPL(Cast);
EQUAL_IMPL(Hash);
EQUAL_IMPL(Array);
EQUAL_IMPL(Literal);
EQUAL_IMPL(UnresolvedConstantLit);
EQUAL_IMPL(ConstantLit);
EQUAL_IMPL(ZSuperArgs);
EQUAL_IMPL(Block);
EQUAL_IMPL(InsSeq);
EQUAL_IMPL(RuntimeMethodDefinition);

} // namespace sorbet::ast
