#include "ast/ast.h"
#include <type_traits>

using namespace std;

namespace sorbet::ast {

namespace {

template <class T> T deepCopyVec(const Expression *avoid, const T &origin) {
    T copy;
    copy.reserve(origin.size());
    for (const auto &memb : origin) {
        copy.emplace_back(memb->_deepCopy(avoid));
    };
    return copy;
}

} // namespace

TreePtr Expression::deepCopy() const {
    TreePtr res;

    try {
        res = this->_deepCopy(this, true);
    } catch (DeepCopyError &e) {
        return nullptr;
    }
    return res;
}

TreePtr ClassDef::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<ClassDef>(this->loc, this->declLoc, this->symbol, this->name->_deepCopy(avoid),
                               deepCopyVec(avoid, this->ancestors), deepCopyVec(avoid, this->rhs), this->kind);
}

TreePtr MethodDef::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<MethodDef>(this->loc, this->declLoc, this->symbol, this->name, deepCopyVec(avoid, this->args),
                                rhs->_deepCopy(avoid), flags);
}

TreePtr If::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<If>(this->loc, this->cond->_deepCopy(avoid), this->thenp->_deepCopy(avoid),
                         this->elsep->_deepCopy(avoid));
}

TreePtr While::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<While>(this->loc, this->cond->_deepCopy(avoid), this->body->_deepCopy(avoid));
}

TreePtr Break::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Break>(this->loc, this->expr->_deepCopy(avoid));
}

TreePtr Retry::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Retry>(this->loc);
}

TreePtr Next::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Next>(this->loc, this->expr->_deepCopy(avoid));
}

TreePtr Return::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Return>(this->loc, this->expr->_deepCopy(avoid));
}

TreePtr RescueCase::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<RescueCase>(this->loc, deepCopyVec(avoid, this->exceptions), var->_deepCopy(avoid),
                                 body->_deepCopy(avoid));
}

TreePtr Rescue::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Rescue>(this->loc, this->body->_deepCopy(avoid),
                             deepCopyVec<Rescue::RESCUE_CASE_store>(avoid, rescueCases), else_->_deepCopy(avoid),
                             ensure->_deepCopy(avoid));
}

TreePtr Local::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Local>(loc, localVariable);
}

TreePtr UnresolvedIdent::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<UnresolvedIdent>(loc, kind, name);
}

TreePtr RestArg::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<RestArg>(loc, expr->_deepCopy(avoid));
}

TreePtr KeywordArg::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<KeywordArg>(loc, expr->_deepCopy(avoid));
}

TreePtr OptionalArg::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<OptionalArg>(loc, expr->_deepCopy(avoid), default_->_deepCopy(avoid));
}

TreePtr BlockArg::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<BlockArg>(loc, expr->_deepCopy(avoid));
}

TreePtr ShadowArg::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<ShadowArg>(loc, expr->_deepCopy(avoid));
}

TreePtr Assign::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Assign>(loc, lhs->_deepCopy(avoid), rhs->_deepCopy(avoid));
}

TreePtr Send::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Send>(loc, recv->_deepCopy(avoid), fun, deepCopyVec(avoid, args),
                           block == nullptr ? nullptr : block->_deepCopy(avoid), flags);
}

TreePtr Cast::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Cast>(loc, type, arg->_deepCopy(avoid), cast);
}

TreePtr Hash::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Hash>(loc, deepCopyVec(avoid, keys), deepCopyVec(avoid, values));
}

TreePtr Array::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Array>(loc, deepCopyVec(avoid, elems));
}

TreePtr Literal::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<Literal>(loc, value);
}

TreePtr UnresolvedConstantLit::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<UnresolvedConstantLit>(loc, scope->_deepCopy(avoid), cnst);
}

TreePtr ConstantLit::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    TreePtr originalC;
    if (original) {
        originalC = original->deepCopy();
    }
    return make_tree<ConstantLit>(this->loc, this->symbol, move(originalC));
}

TreePtr ZSuperArgs::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<ZSuperArgs>(loc);
}

TreePtr Block::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    auto res = make_tree<Block>(loc, deepCopyVec(avoid, args), body->_deepCopy(avoid));
    return res;
}

TreePtr InsSeq::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<InsSeq>(loc, deepCopyVec(avoid, stats), expr->_deepCopy(avoid));
}

TreePtr EmptyTree::_deepCopy(const Expression *avoid, bool root) const {
    if (!root && this == avoid) {
        throw DeepCopyError();
    }
    return make_tree<EmptyTree>();
}
} // namespace sorbet::ast
