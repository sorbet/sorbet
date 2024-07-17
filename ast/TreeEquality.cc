#include "ast/ast.h"
#include <type_traits>

using namespace std;

namespace sorbet::ast {

namespace {

bool structurallyEqual(const void *avoid, const ExpressionPtr &tree, const ExpressionPtr &other, bool root = false);

template <unsigned long N>
bool structurallyEqualVec(const void *avoid, const InlinedVector<ExpressionPtr, N> &a,
                          const InlinedVector<ExpressionPtr, N> &b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (int i = 0; i < a.size(); i++) {
        if (!structurallyEqual(avoid, a[i], b[i])) {
            return false;
        }
    }
    return true;
}

class StructurallyEqualError {};

bool structurallyEqual(const void *avoid, const Tag tag, const void *tree, const void *other, bool root) {
    if (!root && tree == avoid) {
        throw StructurallyEqualError();
    }

    switch (tag) {
        case Tag::EmptyTree:
            return true;

        case Tag::Send: {
            auto *a = reinterpret_cast<const Send *>(tree);
            auto *b = reinterpret_cast<const Send *>(other);
            if (a->fun != b->fun) {
                return false;
            }

            if (a->flags != b->flags) {
                return false;
            }

            if (a->numPosArgs() != b->numPosArgs()) {
                return false;
            }

            if (!structurallyEqual(avoid, a->recv, b->recv)) {
                return false;
            }

            return structurallyEqualVec(avoid, a->rawArgsDoNotUse(), b->rawArgsDoNotUse());
        }

        case Tag::ClassDef: {
            auto *a = reinterpret_cast<const ClassDef *>(tree);
            auto *b = reinterpret_cast<const ClassDef *>(other);
            if (a->symbol != b->symbol) {
                return false;
            }
            if (a->kind != b->kind) {
                return false;
            }
            if (!structurallyEqual(avoid, a->name, b->name)) {
                return false;
            }
            if (!structurallyEqualVec(avoid, a->ancestors, b->ancestors)) {
                return false;
            }
            if (!structurallyEqualVec(avoid, a->singletonAncestors, b->singletonAncestors)) {
                return false;
            }
            if (!structurallyEqualVec(avoid, a->rhs, b->rhs)) {
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
            if (a->flags != b->flags) {
                return false;
            }
            if (!structurallyEqual(avoid, a->rhs, b->rhs)) {
                return false;
            }
            if (!structurallyEqualVec(avoid, a->args, b->args)) {
                return false;
            }
            return true;
        }

        case Tag::If: {
            auto *a = reinterpret_cast<const If *>(tree);
            auto *b = reinterpret_cast<const If *>(other);
            return structurallyEqual(avoid, a->cond, b->cond) && structurallyEqual(avoid, a->thenp, b->thenp) &&
                   structurallyEqual(avoid, a->elsep, b->elsep);
        }

        case Tag::While: {
            auto *a = reinterpret_cast<const While *>(tree);
            auto *b = reinterpret_cast<const While *>(other);
            return structurallyEqual(avoid, a->cond, b->cond) && structurallyEqual(avoid, a->body, b->body);
        }

        case Tag::Break: {
            auto *a = reinterpret_cast<const Break *>(tree);
            auto *b = reinterpret_cast<const Break *>(other);
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::Retry: {
            return true;
        }

        case Tag::Next: {
            auto *a = reinterpret_cast<const Next *>(tree);
            auto *b = reinterpret_cast<const Next *>(other);
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::Return: {
            auto *a = reinterpret_cast<const Return *>(tree);
            auto *b = reinterpret_cast<const Return *>(other);
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::RescueCase: {
            auto *a = reinterpret_cast<const RescueCase *>(tree);
            auto *b = reinterpret_cast<const RescueCase *>(other);
            if (!structurallyEqual(avoid, a->var, b->var)) {
                return false;
            }
            if (!structurallyEqual(avoid, a->body, b->body)) {
                return false;
            }
            return structurallyEqualVec(avoid, a->exceptions, b->exceptions);
        }

        case Tag::Rescue: {
            auto *a = reinterpret_cast<const Rescue *>(tree);
            auto *b = reinterpret_cast<const Rescue *>(other);
            if (!structurallyEqual(avoid, a->body, b->body)) {
                return false;
            }
            if (!structurallyEqual(avoid, a->else_, b->else_)) {
                return false;
            }
            if (!structurallyEqual(avoid, a->ensure, b->ensure)) {
                return false;
            }
            return structurallyEqualVec(avoid, a->rescueCases, b->rescueCases);
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
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::KeywordArg: {
            auto *a = reinterpret_cast<const KeywordArg *>(tree);
            auto *b = reinterpret_cast<const KeywordArg *>(other);
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::OptionalArg: {
            auto *a = reinterpret_cast<const OptionalArg *>(tree);
            auto *b = reinterpret_cast<const OptionalArg *>(other);
            return structurallyEqual(avoid, a->expr, b->expr) && structurallyEqual(avoid, a->default_, b->default_);
        }

        case Tag::BlockArg: {
            auto *a = reinterpret_cast<const BlockArg *>(tree);
            auto *b = reinterpret_cast<const BlockArg *>(other);
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::ShadowArg: {
            auto *a = reinterpret_cast<const ShadowArg *>(tree);
            auto *b = reinterpret_cast<const ShadowArg *>(other);
            return structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::Assign: {
            auto *a = reinterpret_cast<const Assign *>(tree);
            auto *b = reinterpret_cast<const Assign *>(other);
            return structurallyEqual(avoid, a->lhs, b->lhs) && structurallyEqual(avoid, a->rhs, b->rhs);
        }

        case Tag::Cast: {
            auto *a = reinterpret_cast<const Cast *>(tree);
            auto *b = reinterpret_cast<const Cast *>(other);
            if (a->type != b->type) {
                return false;
            }
            if (a->cast != b->cast) {
                return false;
            }
            return structurallyEqual(avoid, a->arg, b->arg) && structurallyEqual(avoid, a->typeExpr, b->typeExpr);
        }

        case Tag::Hash: {
            auto *a = reinterpret_cast<const Hash *>(tree);
            auto *b = reinterpret_cast<const Hash *>(other);
            return structurallyEqualVec(avoid, a->keys, b->keys) && structurallyEqualVec(avoid, a->values, b->values);
        }
        case Tag::Array: {
            auto *a = reinterpret_cast<const Array *>(tree);
            auto *b = reinterpret_cast<const Array *>(other);
            return structurallyEqualVec(avoid, a->elems, b->elems);
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
                ENFORCE(false, "unexpected TypePtr::Tag: {}", aType.tag());
            }
            return false;
        }

        case Tag::UnresolvedConstantLit: {
            auto *a = reinterpret_cast<const UnresolvedConstantLit *>(tree);
            auto *b = reinterpret_cast<const UnresolvedConstantLit *>(other);
            if (a->cnst != b->cnst) {
                return false;
            }
            return structurallyEqual(avoid, a->scope, b->scope);
        }

        case Tag::ConstantLit: {
            auto *a = reinterpret_cast<const ConstantLit *>(tree);
            auto *b = reinterpret_cast<const ConstantLit *>(other);
            return a->symbol == b->symbol && structurallyEqual(avoid, a->original, b->original);
        }

        case Tag::ZSuperArgs: {
            return true;
        }

        case Tag::Block: {
            auto *a = reinterpret_cast<const Block *>(tree);
            auto *b = reinterpret_cast<const Block *>(other);
            return structurallyEqualVec(avoid, a->args, b->args) && structurallyEqual(avoid, a->body, b->body);
        }

        case Tag::InsSeq: {
            auto *a = reinterpret_cast<const InsSeq *>(tree);
            auto *b = reinterpret_cast<const InsSeq *>(other);
            return structurallyEqualVec(avoid, a->stats, b->stats) && structurallyEqual(avoid, a->expr, b->expr);
        }

        case Tag::RuntimeMethodDefinition: {
            auto *a = reinterpret_cast<const RuntimeMethodDefinition *>(tree);
            auto *b = reinterpret_cast<const RuntimeMethodDefinition *>(other);
            return a->name == b->name && a->isSelfMethod == b->isSelfMethod;
        }
    }
}

bool structurallyEqual(const void *avoid, const ExpressionPtr &tree, const ExpressionPtr &other, bool root) {
    ENFORCE(tree != nullptr);
    ENFORCE(other != nullptr);
    if (tree.tag() != other.tag()) {
        return false;
    }

    return structurallyEqual(avoid, tree.tag(), tree.get(), other.get(), root);
}

} // namespace

bool ExpressionPtr::structurallyEqual(const ExpressionPtr &other) const {
    if (tag() != other.tag()) {
        return false;
    }
    try {
        return sorbet::ast::structurallyEqual(get(), tag(), get(), other.get(), true);
    } catch (StructurallyEqualError &e) {
        return false;
    }
}

#define EQUAL_IMPL(name)                                                                                        \
    bool name::structurallyEqual(const ExpressionPtr &other) const {                                            \
        if (ExpressionToTag<name>::value != other.tag()) {                                                      \
            return false;                                                                                       \
        }                                                                                                       \
        try {                                                                                                   \
            return sorbet::ast::structurallyEqual(this, ExpressionToTag<name>::value, this, other.get(), true); \
        } catch (StructurallyEqualError & e) {                                                                  \
            return false;                                                                                       \
        }                                                                                                       \
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
