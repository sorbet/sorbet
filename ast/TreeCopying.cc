#include "ast/ast.h"
#include <type_traits>

using namespace std;

namespace sorbet::ast {

namespace {

TreePtr deepCopy(const Expression *avoid, const TreePtr &tree, bool root = false);

template <class T> T deepCopyVec(const Expression *avoid, const T &origin) {
    T copy;
    copy.reserve(origin.size());
    for (const auto &memb : origin) {
        copy.emplace_back(deepCopy(avoid, memb));
    };
    return copy;
}

class DeepCopyError {};

TreePtr deepCopy(const Expression *avoid, const Tag tag, const Expression *tree, bool root) {
    if (!root && tree == avoid) {
        throw DeepCopyError();
    }

    switch (tag) {
        case Tag::EmptyTree:
            return make_tree<EmptyTree>();

        case Tag::Send: {
            auto *exp = reinterpret_cast<const Send *>(tree);
            return make_tree<Send>(exp->loc, deepCopy(avoid, exp->recv), exp->fun, exp->numPosArgs,
                                   deepCopyVec(avoid, exp->args),
                                   exp->block == nullptr ? nullptr : deepCopy(avoid, exp->block), exp->flags);
        }

        case Tag::ClassDef: {
            auto *exp = reinterpret_cast<const ClassDef *>(tree);
            return make_tree<ClassDef>(exp->loc, exp->declLoc, exp->symbol, deepCopy(avoid, exp->name),
                                       deepCopyVec(avoid, exp->ancestors), deepCopyVec(avoid, exp->rhs), exp->kind);
        }

        case Tag::MethodDef: {
            auto *exp = reinterpret_cast<const MethodDef *>(tree);
            return make_tree<MethodDef>(exp->loc, exp->declLoc, exp->symbol, exp->name, deepCopyVec(avoid, exp->args),
                                        deepCopy(avoid, exp->rhs), exp->flags);
        }

        case Tag::If: {
            auto *exp = reinterpret_cast<const If *>(tree);
            return make_tree<If>(exp->loc, deepCopy(avoid, exp->cond), deepCopy(avoid, exp->thenp),
                                 deepCopy(avoid, exp->elsep));
        }

        case Tag::While: {
            auto *exp = reinterpret_cast<const While *>(tree);
            return make_tree<While>(exp->loc, deepCopy(avoid, exp->cond), deepCopy(avoid, exp->body));
        }

        case Tag::Break: {
            auto *exp = reinterpret_cast<const Break *>(tree);
            return make_tree<Break>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::Retry: {
            auto *exp = reinterpret_cast<const Retry *>(tree);
            return make_tree<Retry>(exp->loc);
        }

        case Tag::Next: {
            auto *exp = reinterpret_cast<const Next *>(tree);
            return make_tree<Next>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::Return: {
            auto *exp = reinterpret_cast<const Return *>(tree);
            return make_tree<Return>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::RescueCase: {
            auto *exp = reinterpret_cast<const RescueCase *>(tree);
            return make_tree<RescueCase>(exp->loc, deepCopyVec(avoid, exp->exceptions), deepCopy(avoid, exp->var),
                                         deepCopy(avoid, exp->body));
        }

        case Tag::Rescue: {
            auto *exp = reinterpret_cast<const Rescue *>(tree);
            return make_tree<Rescue>(exp->loc, deepCopy(avoid, exp->body),
                                     deepCopyVec<Rescue::RESCUE_CASE_store>(avoid, exp->rescueCases),
                                     deepCopy(avoid, exp->else_), deepCopy(avoid, exp->ensure));
        }

        case Tag::Local: {
            auto *exp = reinterpret_cast<const Local *>(tree);
            return make_tree<Local>(exp->loc, exp->localVariable);
        }

        case Tag::UnresolvedIdent: {
            auto *exp = reinterpret_cast<const UnresolvedIdent *>(tree);
            return make_tree<UnresolvedIdent>(exp->loc, exp->kind, exp->name);
        }

        case Tag::RestArg: {
            auto *exp = reinterpret_cast<const RestArg *>(tree);
            return make_tree<RestArg>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::KeywordArg: {
            auto *exp = reinterpret_cast<const KeywordArg *>(tree);
            return make_tree<KeywordArg>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::OptionalArg: {
            auto *exp = reinterpret_cast<const OptionalArg *>(tree);
            return make_tree<OptionalArg>(exp->loc, deepCopy(avoid, exp->expr), deepCopy(avoid, exp->default_));
        }

        case Tag::BlockArg: {
            auto *exp = reinterpret_cast<const BlockArg *>(tree);
            return make_tree<BlockArg>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::ShadowArg: {
            auto *exp = reinterpret_cast<const ShadowArg *>(tree);
            return make_tree<ShadowArg>(exp->loc, deepCopy(avoid, exp->expr));
        }

        case Tag::Assign: {
            auto *exp = reinterpret_cast<const Assign *>(tree);
            return make_tree<Assign>(exp->loc, deepCopy(avoid, exp->lhs), deepCopy(avoid, exp->rhs));
        }

        case Tag::Cast: {
            auto *exp = reinterpret_cast<const Cast *>(tree);
            return make_tree<Cast>(exp->loc, exp->type, deepCopy(avoid, exp->arg), exp->cast);
        }

        case Tag::Hash: {
            auto *exp = reinterpret_cast<const Hash *>(tree);
            return make_tree<Hash>(exp->loc, deepCopyVec(avoid, exp->keys), deepCopyVec(avoid, exp->values));
        }

        case Tag::Array: {
            auto *exp = reinterpret_cast<const Array *>(tree);
            return make_tree<Array>(exp->loc, deepCopyVec(avoid, exp->elems));
        }

        case Tag::Literal: {
            auto *exp = reinterpret_cast<const Literal *>(tree);
            return make_tree<Literal>(exp->loc, exp->value);
        }

        case Tag::UnresolvedConstantLit: {
            auto *exp = reinterpret_cast<const UnresolvedConstantLit *>(tree);
            return make_tree<UnresolvedConstantLit>(exp->loc, deepCopy(avoid, exp->scope), exp->cnst);
        }

        case Tag::ConstantLit: {
            auto *exp = reinterpret_cast<const ConstantLit *>(tree);
            TreePtr originalC;
            if (exp->original) {
                originalC = deepCopy(avoid, exp->original);
            }
            return make_tree<ConstantLit>(exp->loc, exp->symbol, move(originalC));
        }

        case Tag::ZSuperArgs: {
            auto *exp = reinterpret_cast<const ZSuperArgs *>(tree);
            return make_tree<ZSuperArgs>(exp->loc);
        }

        case Tag::Block: {
            auto *exp = reinterpret_cast<const Block *>(tree);
            return make_tree<Block>(exp->loc, deepCopyVec(avoid, exp->args), deepCopy(avoid, exp->body));
        }

        case Tag::InsSeq: {
            auto *exp = reinterpret_cast<const InsSeq *>(tree);
            return make_tree<InsSeq>(exp->loc, deepCopyVec(avoid, exp->stats), deepCopy(avoid, exp->expr));
        }
    }
}

TreePtr deepCopy(const Expression *avoid, const TreePtr &tree, bool root) {
    ENFORCE(tree != nullptr);

    return deepCopy(avoid, tree.tag(), tree.get(), root);
}

} // namespace

TreePtr TreePtr::deepCopy() const {
    try {
        return sorbet::ast::deepCopy(get(), tag(), get(), true);
    } catch (DeepCopyError &e) {
        return nullptr;
    }
}

#define COPY_IMPL(name)                                                             \
    TreePtr name::deepCopy() const {                                                \
        try {                                                                       \
            return sorbet::ast::deepCopy(this, TreeToTag<name>::value, this, true); \
        } catch (DeepCopyError & e) {                                               \
            return nullptr;                                                         \
        }                                                                           \
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

} // namespace sorbet::ast
