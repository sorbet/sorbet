#include "ast/ast.h"
#include <type_traits>

using namespace std;

namespace sorbet::ast {

namespace {

ExpressionPtr deepCopy(const void *avoid, const ExpressionPtr &tree, bool root = false);

template <class T> T deepCopyVec(const void *avoid, const T &origin) {
    T copy;
    copy.reserve(origin.size());
    for (const auto &memb : origin) {
        copy.emplace_back(deepCopy(avoid, memb));
    };
    return copy;
}

}

Send::Send(const void *avoid, const Send *exp)
    : loc(exp->loc), fun(exp->fun), funLoc(exp->funLoc), flags(exp->flags),
      numPosArgs_(exp->numPosArgs_), recv(ast::deepCopy(avoid, exp->recv)),
      args(deepCopyVec(avoid, exp->args)) {
}

ClassDef::ClassDef(const void *avoid, const ClassDef *exp)
    : loc(exp->loc), declLoc(exp->declLoc), symbol(exp->symbol),
      kind(exp->kind), rhs(deepCopyVec(avoid, exp->rhs)), name(ast::deepCopy(avoid, exp->name)), ancestors(deepCopyVec(avoid, exp->ancestors)) {
}

MethodDef::MethodDef(const void *avoid, const MethodDef *exp)
    : loc(exp->loc), declLoc(exp->declLoc), symbol(exp->symbol),
      rhs(ast::deepCopy(avoid, exp->rhs)), args(deepCopyVec(avoid, exp->args)),
      name(exp->name), flags(exp->flags) {
}

If::If(const void *avoid, const If *exp)
    : loc(exp->loc), cond(ast::deepCopy(avoid, exp->cond)), thenp(ast::deepCopy(avoid, exp->thenp)),
        elsep(ast::deepCopy(avoid, exp->elsep)) {
}

While::While(const void *avoid, const While *exp)
    : loc(exp->loc), cond(ast::deepCopy(avoid, exp->cond)), body(ast::deepCopy(avoid, exp->body)) {
}

Break::Break(const void *avoid, const Break *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

Retry::Retry(const void *avoid, const Retry *exp)
    : loc(exp->loc) {
}

Next::Next(const void *avoid, const Next *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

Return::Return(const void *avoid, const Return *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

RescueCase::RescueCase(const void *avoid, const RescueCase *exp)
    : loc(exp->loc), exceptions(deepCopyVec(avoid, exp->exceptions)), var(ast::deepCopy(avoid, exp->var)), body(ast::deepCopy(avoid, exp->body)) {
}

Rescue::Rescue(const void *avoid, const Rescue *exp)
    : loc(exp->loc), body(ast::deepCopy(avoid, exp->body)), rescueCases(deepCopyVec<Rescue::RESCUE_CASE_store>(avoid, exp->rescueCases)),
      else_(ast::deepCopy(avoid, exp->else_)), ensure(ast::deepCopy(avoid, exp->ensure)) {
}

Local::Local(const void *avoid, const Local *exp)
    : loc(exp->loc), localVariable(exp->localVariable) {
}

UnresolvedIdent::UnresolvedIdent(const void *avoid, const UnresolvedIdent *exp)
    : loc(exp->loc), name(exp->name), kind(exp->kind) {
}

RestArg::RestArg(const void *avoid, const RestArg *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

KeywordArg::KeywordArg(const void *avoid, const KeywordArg *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

OptionalArg::OptionalArg(const void *avoid, const OptionalArg *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)), default_(ast::deepCopy(avoid, exp->default_)) {
}

BlockArg::BlockArg(const void *avoid, const BlockArg *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

ShadowArg::ShadowArg(const void *avoid, const ShadowArg *exp)
    : loc(exp->loc), expr(ast::deepCopy(avoid, exp->expr)) {
}

Assign::Assign(const void *avoid, const Assign *exp)
    : loc(exp->loc), lhs(ast::deepCopy(avoid, exp->lhs)), rhs(ast::deepCopy(avoid, exp->rhs)) {
}

Cast::Cast(const void *avoid, const Cast *exp)
    : loc(exp->loc), cast(exp->cast), type(exp->type), arg(ast::deepCopy(avoid, exp->arg)) {
}

Hash::Hash(const void *avoid, const Hash *exp)
    : loc(exp->loc), keys(deepCopyVec(avoid, exp->keys)), values(deepCopyVec(avoid, exp->values)) {
}

Array::Array(const void *avoid, const Array *exp)
    : loc(exp->loc), elems(deepCopyVec(avoid, exp->elems)) {
}

Literal::Literal(const void *avoid, const Literal *exp)
    : loc(exp->loc), value(exp->value) {
}

UnresolvedConstantLit::UnresolvedConstantLit(const void *avoid, const UnresolvedConstantLit *exp)
    : loc(exp->loc), cnst(exp->cnst), scope(ast::deepCopy(avoid, exp->scope)) {
}

ConstantLit::ConstantLit(const void *avoid, const ConstantLit *exp)
    : loc(exp->loc), symbol(exp->symbol), original(exp->original == nullptr ? nullptr : ast::deepCopy(avoid, exp->original)) {
}

ZSuperArgs::ZSuperArgs(const void *avoid, const ZSuperArgs *exp)
    : loc(exp->loc) {
}

Block::Block(const void *avoid, const Block *exp)
    : loc(exp->loc), args(deepCopyVec(avoid, exp->args)), body(ast::deepCopy(avoid, exp->body)) {
}

InsSeq::InsSeq(const void *avoid, const InsSeq *exp)
    : loc(exp->loc), stats(deepCopyVec(avoid, exp->stats)), expr(ast::deepCopy(avoid, exp->expr)) {
}

RuntimeMethodDefinition::RuntimeMethodDefinition(const void *avoid, const RuntimeMethodDefinition *exp)
    : loc(exp->loc), name(exp->name), isSelfMethod(exp->isSelfMethod) {
}

class TreeCopier {
public:
    template <class E>
    static ExpressionPtr copy(const void *avoid, const E *exp) {
        return ExpressionPtr(ExpressionToTag<E>::value, new E(avoid, exp));
    }
};

namespace {

class DeepCopyError {};

ExpressionPtr deepCopy(const void *avoid, const Tag tag, const void *tree, bool root) {
    if (!root && tree == avoid) {
        throw DeepCopyError();
    }

    switch (tag) {
        case Tag::EmptyTree:
            return make_expression<EmptyTree>();

        case Tag::Send: {
            auto *exp = reinterpret_cast<const Send *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::ClassDef: {
            auto *exp = reinterpret_cast<const ClassDef *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::MethodDef: {
            auto *exp = reinterpret_cast<const MethodDef *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::If: {
            auto *exp = reinterpret_cast<const If *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::While: {
            auto *exp = reinterpret_cast<const While *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Break: {
            auto *exp = reinterpret_cast<const Break *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Retry: {
            auto *exp = reinterpret_cast<const Retry *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Next: {
            auto *exp = reinterpret_cast<const Next *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Return: {
            auto *exp = reinterpret_cast<const Return *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::RescueCase: {
            auto *exp = reinterpret_cast<const RescueCase *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Rescue: {
            auto *exp = reinterpret_cast<const Rescue *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Local: {
            auto *exp = reinterpret_cast<const Local *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::UnresolvedIdent: {
            auto *exp = reinterpret_cast<const UnresolvedIdent *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::RestArg: {
            auto *exp = reinterpret_cast<const RestArg *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::KeywordArg: {
            auto *exp = reinterpret_cast<const KeywordArg *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::OptionalArg: {
            auto *exp = reinterpret_cast<const OptionalArg *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::BlockArg: {
            auto *exp = reinterpret_cast<const BlockArg *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::ShadowArg: {
            auto *exp = reinterpret_cast<const ShadowArg *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Assign: {
            auto *exp = reinterpret_cast<const Assign *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Cast: {
            auto *exp = reinterpret_cast<const Cast *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Hash: {
            auto *exp = reinterpret_cast<const Hash *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Array: {
            auto *exp = reinterpret_cast<const Array *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Literal: {
            auto *exp = reinterpret_cast<const Literal *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::UnresolvedConstantLit: {
            auto *exp = reinterpret_cast<const UnresolvedConstantLit *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::ConstantLit: {
            auto *exp = reinterpret_cast<const ConstantLit *>(tree);
            ExpressionPtr originalC;
            if (exp->original) {
                originalC = deepCopy(avoid, exp->original);
            }
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::ZSuperArgs: {
            auto *exp = reinterpret_cast<const ZSuperArgs *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::Block: {
            auto *exp = reinterpret_cast<const Block *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::InsSeq: {
            auto *exp = reinterpret_cast<const InsSeq *>(tree);
            return TreeCopier::copy(avoid, exp);
        }

        case Tag::RuntimeMethodDefinition: {
            auto *exp = reinterpret_cast<const RuntimeMethodDefinition *>(tree);
            return TreeCopier::copy(avoid, exp);
        }
    }
}

ExpressionPtr deepCopy(const void *avoid, const ExpressionPtr &tree, bool root) {
    ENFORCE(tree != nullptr);

    return deepCopy(avoid, tree.tag(), tree.get(), root);
}

} // namespace

ExpressionPtr ExpressionPtr::deepCopy() const {
    try {
        return sorbet::ast::deepCopy(get(), tag(), get(), true);
    } catch (DeepCopyError &e) {
        return nullptr;
    }
}

#define COPY_IMPL(name)                                                                   \
    ExpressionPtr name::deepCopy() const {                                                \
        try {                                                                             \
            return sorbet::ast::deepCopy(this, ExpressionToTag<name>::value, this, true); \
        } catch (DeepCopyError & e) {                                                     \
            return nullptr;                                                               \
        }                                                                                 \
    }

COPY_IMPL(EmptyTree);
COPY_IMPL(Send);
COPY_IMPL(ClassDef);
COPY_IMPL(MethodDef);
COPY_IMPL(If);
COPY_IMPL(While);
COPY_IMPL(Break);
COPY_IMPL(Retry);
COPY_IMPL(Next);
COPY_IMPL(Return);
COPY_IMPL(RescueCase);
COPY_IMPL(Rescue);
COPY_IMPL(Local);
COPY_IMPL(UnresolvedIdent);
COPY_IMPL(RestArg);
COPY_IMPL(KeywordArg);
COPY_IMPL(OptionalArg);
COPY_IMPL(BlockArg);
COPY_IMPL(ShadowArg);
COPY_IMPL(Assign);
COPY_IMPL(Cast);
COPY_IMPL(Hash);
COPY_IMPL(Array);
COPY_IMPL(Literal);
COPY_IMPL(UnresolvedConstantLit);
COPY_IMPL(ConstantLit);
COPY_IMPL(ZSuperArgs);
COPY_IMPL(Block);
COPY_IMPL(InsSeq);
COPY_IMPL(RuntimeMethodDefinition);

} // namespace sorbet::ast
