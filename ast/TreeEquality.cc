#include "absl/algorithm/container.h"
#include "absl/strings/escaping.h"
#include "absl/types/span.h"
#include "ast/ast.h"
#include <type_traits>

using namespace std;

namespace sorbet::ast {

namespace {

template <typename Comparator>
bool compareTrees(const core::GlobalState &gs, const void *avoid, const ExpressionPtr &tree, const ExpressionPtr &other,
                  const core::FileRef file, bool root = false);

template <typename Comparator>
bool compareTreeSpans(const core::GlobalState &gs, const void *avoid, absl::Span<const ExpressionPtr> a,
                      absl::Span<const ExpressionPtr> b, const core::FileRef file) {
    return absl::c_equal(a, b,
                         [&](const auto &a, const auto &b) { return compareTrees<Comparator>(gs, avoid, a, b, file); });
}

class StructurallyEqualError {};

template <typename Comparator>
bool compareTrees(const core::GlobalState &gs, const void *avoid, const Tag tag, const void *tree, const void *other,
                  const core::FileRef file, bool root) {
    if (!root && tree == avoid) {
        throw StructurallyEqualError();
    }

    switch (tag) {
        case Tag::EmptyTree:
            return true;

        case Tag::Send: {
            auto *a = reinterpret_cast<const Send *>(tree);
            auto *b = reinterpret_cast<const Send *>(other);
            if (!Comparator::compareNames(gs, a->fun, b->fun)) {
                return false;
            }

            if (a->flags != b->flags) {
                return false;
            }

            if (a->numPosArgs() != b->numPosArgs()) {
                return false;
            }

            if (!Comparator::compareNodes(gs, avoid, a->recv, b->recv, file)) {
                return false;
            }

            return Comparator::compareSpans(gs, avoid, a->rawArgsDoNotUse(), b->rawArgsDoNotUse(), file);
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
            if (!Comparator::compareNodes(gs, avoid, a->name, b->name, file)) {
                return false;
            }
            if (!Comparator::compareSpans(gs, avoid, a->ancestors, b->ancestors, file)) {
                return false;
            }
            if (!Comparator::compareSpans(gs, avoid, a->singletonAncestors, b->singletonAncestors, file)) {
                return false;
            }
            if (!Comparator::compareSpans(gs, avoid, a->rhs, b->rhs, file)) {
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
            if (!Comparator::compareNames(gs, a->name, b->name)) {
                return false;
            }
            if (a->flags != b->flags) {
                return false;
            }
            if (!Comparator::compareNodes(gs, avoid, a->rhs, b->rhs, file)) {
                return false;
            }
            if (!Comparator::compareSpans(gs, avoid, a->params, b->params, file)) {
                return false;
            }
            return true;
        }

        case Tag::If: {
            auto *a = reinterpret_cast<const If *>(tree);
            auto *b = reinterpret_cast<const If *>(other);
            return Comparator::compareNodes(gs, avoid, a->cond, b->cond, file) &&
                   Comparator::compareNodes(gs, avoid, a->thenp, b->thenp, file) &&
                   Comparator::compareNodes(gs, avoid, a->elsep, b->elsep, file);
        }

        case Tag::While: {
            auto *a = reinterpret_cast<const While *>(tree);
            auto *b = reinterpret_cast<const While *>(other);
            return Comparator::compareNodes(gs, avoid, a->cond, b->cond, file) &&
                   Comparator::compareNodes(gs, avoid, a->body, b->body, file);
        }

        case Tag::Break: {
            auto *a = reinterpret_cast<const Break *>(tree);
            auto *b = reinterpret_cast<const Break *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::Retry: {
            return true;
        }

        case Tag::Next: {
            auto *a = reinterpret_cast<const Next *>(tree);
            auto *b = reinterpret_cast<const Next *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::Return: {
            auto *a = reinterpret_cast<const Return *>(tree);
            auto *b = reinterpret_cast<const Return *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::RescueCase: {
            auto *a = reinterpret_cast<const RescueCase *>(tree);
            auto *b = reinterpret_cast<const RescueCase *>(other);
            if (!Comparator::compareNodes(gs, avoid, a->var, b->var, file)) {
                return false;
            }
            if (!Comparator::compareNodes(gs, avoid, a->body, b->body, file)) {
                return false;
            }
            return Comparator::compareSpans(gs, avoid, a->exceptions, b->exceptions, file);
        }

        case Tag::Rescue: {
            auto *a = reinterpret_cast<const Rescue *>(tree);
            auto *b = reinterpret_cast<const Rescue *>(other);
            if (!Comparator::compareNodes(gs, avoid, a->body, b->body, file)) {
                return false;
            }
            if (!Comparator::compareNodes(gs, avoid, a->else_, b->else_, file)) {
                return false;
            }
            if (!Comparator::compareNodes(gs, avoid, a->ensure, b->ensure, file)) {
                return false;
            }
            return Comparator::compareSpans(gs, avoid, a->rescueCases, b->rescueCases, file);
        }

        case Tag::Local: {
            auto *a = reinterpret_cast<const Local *>(tree);
            auto *b = reinterpret_cast<const Local *>(other);
            return a->localVariable == b->localVariable;
        }

        case Tag::UnresolvedIdent: {
            auto *a = reinterpret_cast<const UnresolvedIdent *>(tree);
            auto *b = reinterpret_cast<const UnresolvedIdent *>(other);
            return a->kind == b->kind && Comparator::compareNames(gs, a->name, b->name);
        }

        case Tag::RestParam: {
            auto *a = reinterpret_cast<const RestParam *>(tree);
            auto *b = reinterpret_cast<const RestParam *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::KeywordArg: {
            auto *a = reinterpret_cast<const KeywordArg *>(tree);
            auto *b = reinterpret_cast<const KeywordArg *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::OptionalParam: {
            auto *a = reinterpret_cast<const OptionalParam *>(tree);
            auto *b = reinterpret_cast<const OptionalParam *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file) &&
                   Comparator::compareNodes(gs, avoid, a->default_, b->default_, file);
        }

        case Tag::BlockArg: {
            auto *a = reinterpret_cast<const BlockArg *>(tree);
            auto *b = reinterpret_cast<const BlockArg *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::ShadowArg: {
            auto *a = reinterpret_cast<const ShadowArg *>(tree);
            auto *b = reinterpret_cast<const ShadowArg *>(other);
            return Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::Assign: {
            auto *a = reinterpret_cast<const Assign *>(tree);
            auto *b = reinterpret_cast<const Assign *>(other);
            return Comparator::compareNodes(gs, avoid, a->lhs, b->lhs, file) &&
                   Comparator::compareNodes(gs, avoid, a->rhs, b->rhs, file);
        }

        case Tag::Cast: {
            auto *a = reinterpret_cast<const Cast *>(tree);
            auto *b = reinterpret_cast<const Cast *>(other);
            if (a->type != b->type) {
                return false;
            }
            if (!Comparator::compareNames(gs, a->cast, b->cast)) {
                return false;
            }
            return Comparator::compareNodes(gs, avoid, a->arg, b->arg, file) &&
                   Comparator::compareNodes(gs, avoid, a->typeExpr, b->typeExpr, file);
        }

        case Tag::Hash: {
            auto *a = reinterpret_cast<const Hash *>(tree);
            auto *b = reinterpret_cast<const Hash *>(other);
            return Comparator::compareSpans(gs, avoid, a->keys, b->keys, file) &&
                   Comparator::compareSpans(gs, avoid, a->values, b->values, file);
        }
        case Tag::Array: {
            auto *a = reinterpret_cast<const Array *>(tree);
            auto *b = reinterpret_cast<const Array *>(other);
            return Comparator::compareSpans(gs, avoid, a->elems, b->elems, file);
        }

        case Tag::Literal: {
            auto *a = reinterpret_cast<const Literal *>(tree);
            auto *b = reinterpret_cast<const Literal *>(other);
            const auto &aType = a->value;
            const auto &bType = b->value;
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
                return named_literal_a.kind == named_literal_b.kind &&
                       Comparator::compareNames(gs, named_literal_a.name, named_literal_b.name);
            } else if (aType.tag() == core::TypePtr::Tag::ClassType) {
                auto class_type_a = core::cast_type_nonnull<core::ClassType>(aType);
                auto class_type_b = core::cast_type_nonnull<core::ClassType>(bType);
                return class_type_a.symbol == class_type_b.symbol;
            } else {
                ENFORCE(false, "unexpected TypePtr::Tag: {}", fmt::underlying(aType.tag()));
            }
            return false;
        }

        case Tag::UnresolvedConstantLit: {
            auto *a = reinterpret_cast<const UnresolvedConstantLit *>(tree);
            auto *b = reinterpret_cast<const UnresolvedConstantLit *>(other);
            if (!Comparator::compareNames(gs, a->cnst, b->cnst)) {
                return false;
            }
            return Comparator::compareNodes(gs, avoid, a->scope, b->scope, file);
        }

        case Tag::ConstantLit: {
            auto *a = reinterpret_cast<const ConstantLit *>(tree);
            auto *b = reinterpret_cast<const ConstantLit *>(other);
            if (a->symbol() != b->symbol()) {
                return false;
            }
            if (a->original() && b->original()) {
                auto &alit = *a->original();
                auto &blit = *b->original();
                if (alit.cnst != blit.cnst) {
                    return false;
                }
                return Comparator::compareNodes(gs, avoid, alit.scope, blit.scope, file);
            } else if (!a->original() && !b->original()) {
                // This occurs when the constant is created using MK::Constant instead of MK::UnresolvedConstant
                // (original points to the UnresolvedConstantLit that created this ConstantLit)
                return true;
            } else {
                return false;
            }
        }

        case Tag::ZSuperArgs: {
            return true;
        }

        case Tag::Block: {
            auto *a = reinterpret_cast<const Block *>(tree);
            auto *b = reinterpret_cast<const Block *>(other);
            return Comparator::compareSpans(gs, avoid, a->params, b->params, file) &&
                   Comparator::compareNodes(gs, avoid, a->body, b->body, file);
        }

        case Tag::InsSeq: {
            auto *a = reinterpret_cast<const InsSeq *>(tree);
            auto *b = reinterpret_cast<const InsSeq *>(other);
            return Comparator::compareSpans(gs, avoid, a->stats, b->stats, file) &&
                   Comparator::compareNodes(gs, avoid, a->expr, b->expr, file);
        }

        case Tag::RuntimeMethodDefinition: {
            auto *a = reinterpret_cast<const RuntimeMethodDefinition *>(tree);
            auto *b = reinterpret_cast<const RuntimeMethodDefinition *>(other);
            return Comparator::compareNames(gs, a->name, b->name) && a->isSelfMethod == b->isSelfMethod;
        }

        case Tag::Self: {
            return true;
        }
    }
}

template <typename Comparator>
bool compareTrees(const core::GlobalState &gs, const void *avoid, const ExpressionPtr &tree, const ExpressionPtr &other,
                  const core::FileRef file, bool root) {
    ENFORCE(tree);
    ENFORCE(other);
    if (!tree) {
        fatalLogger->error("msg=\"ExtractToVariable: tree is null in structurallyEqual\"");
    }
    if (!other) {
        fatalLogger->error("msg=\"ExtractToVariable: other is null in structurallyEqual\"");
    }
    if (!tree || !other) {
        fatalLogger->error("filename=\"{}\" source=\"{}\"", file.data(gs).path(),
                           absl::CEscape(file.data(gs).source()));
        return false;
    }
    if (tree.tag() != other.tag()) {
        return false;
    }

    return compareTrees<Comparator>(gs, avoid, tree.tag(), tree.get(), other.get(), file, root);
}

struct StructurallyEqualComparator {
    static bool compareNodes(const core::GlobalState &gs, const void *avoid, const ExpressionPtr &tree,
                             const ExpressionPtr &other, const core::FileRef file) {
        return compareTrees<StructurallyEqualComparator>(gs, avoid, tree, other, file);
    }

    static bool compareSpans(const core::GlobalState &gs, const void *avoid, absl::Span<const ExpressionPtr> a,
                             absl::Span<const ExpressionPtr> b, const core::FileRef file) {
        return compareTreeSpans<StructurallyEqualComparator>(gs, avoid, a, b, file);
    }

    static bool compareNames(const core::GlobalState &gs, const core::NameRef &a, const core::NameRef &b) {
        return a == b;
    }
};

// TODO: Clean up after Prism work is done. https://github.com/sorbet/sorbet/issues/9065
struct PrismDesugarComparator {
    static bool compareNodes(const core::GlobalState &gs, const void *avoid, const ExpressionPtr &tree,
                             const ExpressionPtr &other, const core::FileRef file) {
        if (tree.loc() != other.loc()) {
            return false;
        }

        return compareTrees<PrismDesugarComparator>(gs, avoid, tree, other, file);
    }

    static bool compareSpans(const core::GlobalState &gs, const void *avoid, absl::Span<const ExpressionPtr> a,
                             absl::Span<const ExpressionPtr> b, const core::FileRef file) {
        return compareTreeSpans<PrismDesugarComparator>(gs, avoid, a, b, file);
    }

    static bool compareNames(const core::GlobalState &gs, const core::NameRef &a, const core::NameRef &b) {
        if (a == b) {
            return true;
        }

        // During the migration, unqiue names are generated from two sources:
        //   1. `Translator.desugarUniqueCounter` in `parser/prism/Translator.cc`
        //   2. `DesugarContext.uniqueCounter`    in `ast/desugar/PrismDesugar.cc`
        //
        // There's no easy way to synchronize them:
        //    * Any nodes that get directly desugared directly by the translator will cause the `PrismDesugar.cc`
        //      counter to *not* increment, causing all of its subsequent unique names to be wrong.
        //    * Conversly, any nodes that hit the fallback path will cause increment the `PrismDesugar.cc` counter,
        //      but have no easy way to incrementing all the alraady-created names.
        //
        // So for now, we'll ignore the difference between uniquely generated names.
        return a.hasUniqueNameKind(gs, core::UniqueNameKind::Desugar) &&
               b.hasUniqueNameKind(gs, core::UniqueNameKind::Desugar);
    }
};

} // namespace

bool ExpressionPtr::structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other,
                                      const core::FileRef file) const {
    if (tag() != other.tag()) {
        return false;
    }
    try {
        return sorbet::ast::compareTrees<StructurallyEqualComparator>(gs, get(), tag(), get(), other.get(), file, true);
    } catch (StructurallyEqualError &e) {
        return false;
    }
}

bool ExpressionPtr::prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other,
                                      const core::FileRef file) const {
    if (tag() != other.tag()) {
        return false;
    }
    try {
        return sorbet::ast::compareTrees<PrismDesugarComparator>(gs, get(), tag(), get(), other.get(), file, true);
    } catch (StructurallyEqualError &e) {
        return false;
    }
}

#define EQUAL_IMPL(name)                                                                                            \
    bool name::structurallyEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) \
        const {                                                                                                     \
        if (ExpressionToTag<name>::value != other.tag()) {                                                          \
            return false;                                                                                           \
        }                                                                                                           \
        try {                                                                                                       \
            return sorbet::ast::compareTrees<StructurallyEqualComparator>(gs, this, ExpressionToTag<name>::value,   \
                                                                          this, other.get(), file, true);           \
        } catch (StructurallyEqualError & e) {                                                                      \
            return false;                                                                                           \
        }                                                                                                           \
    }                                                                                                               \
                                                                                                                    \
    bool name::prismDesugarEqual(const core::GlobalState &gs, const ExpressionPtr &other, const core::FileRef file) \
        const {                                                                                                     \
        if (ExpressionToTag<name>::value != other.tag()) {                                                          \
            return false;                                                                                           \
        }                                                                                                           \
        try {                                                                                                       \
            return sorbet::ast::compareTrees<PrismDesugarComparator>(gs, this, ExpressionToTag<name>::value, this,  \
                                                                     other.get(), file, true);                      \
        } catch (StructurallyEqualError & e) {                                                                      \
            return false;                                                                                           \
        }                                                                                                           \
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
EQUAL_IMPL(RestParam);
EQUAL_IMPL(KeywordArg);
EQUAL_IMPL(OptionalParam);
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
