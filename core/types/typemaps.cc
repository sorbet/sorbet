#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;
namespace sorbet::core {

TypePtr Types::instantiate(const GlobalState &gs, const TypePtr &what, const InlinedVector<SymbolRef, 4> &params,
                           const vector<TypePtr> &targs) {
    ENFORCE(what != nullptr);
    auto t = what._instantiate(gs, params, targs);
    if (t) {
        return t;
    }
    return what;
}

TypePtr Types::instantiate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(tc.isSolved());
    if (tc.isEmpty()) {
        return what;
    }
    ENFORCE(what != nullptr);
    auto t = what._instantiate(gs, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr Types::approximate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(what != nullptr);
    auto t = what._approximate(gs, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr TypeVar::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    return tc.getInstantiation(sym);
}

TypePtr TypeVar::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    if (tc.hasUpperBound(sym)) {
        auto bound = tc.findUpperBound(sym);
        if (bound.isFullyDefined()) {
            return bound;
        }
    }
    // TODO: in many languages this method is a huge adhoc heuristic
    // let's see if we can keep it small
    return Types::top();
}

namespace {
template <typename... TransformArgs>
optional<vector<TypePtr>> mungeArgs(const vector<TypePtr> &elems,
                                    TypePtr (TypePtr::*method)(const TransformArgs &...) const,
                                    const TransformArgs &... transformArgs) {
    optional<vector<TypePtr>> newArgs;
    int i = -1;
    for (auto &e : elems) {
        ++i;
        auto t = (e.*method)(transformArgs...);
        if (!newArgs.has_value() && !t) {
            continue;
        }

        if (!newArgs.has_value()) {
            // Oops, need to fixup all the elements that should be there.
            newArgs.emplace();
            newArgs->reserve(elems.size());
            for (int j = 0; j < i; ++j) {
                newArgs->emplace_back(elems[j]);
            }
        }

        if (!t) {
            t = e;
        }

        ENFORCE(newArgs->size() == i);
        newArgs->emplace_back(t);
    }
    return newArgs;
}
} // anonymous namespace

TypePtr TupleType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                const vector<TypePtr> &targs) const {
    optional<vector<TypePtr>> newElems = mungeArgs(this->elems, &TypePtr::_instantiate, gs, params, targs);
    if (!newElems) {
        return nullptr;
    }
    return TupleType::build(gs, move(*newElems));
}

TypePtr TupleType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newElems = mungeArgs(this->elems, &TypePtr::_instantiate, gs, tc);
    if (!newElems) {
        return nullptr;
    }
    return TupleType::build(gs, move(*newElems));
}

TypePtr TupleType::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newElems = mungeArgs(this->elems, &TypePtr::_approximate, gs, tc);
    if (!newElems) {
        return nullptr;
    }
    return TupleType::build(gs, move(*newElems));
};

TypePtr ShapeType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                const vector<TypePtr> &targs) const {
    optional<vector<TypePtr>> newValues = mungeArgs(this->values, &TypePtr::_instantiate, gs, params, targs);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, move(*newValues));
}

TypePtr ShapeType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newValues = mungeArgs(this->values, &TypePtr::_instantiate, gs, tc);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, move(*newValues));
}

TypePtr ShapeType::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newValues = mungeArgs(this->values, &TypePtr::_approximate, gs, tc);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, move(*newValues));
}

TypePtr OrType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                             const vector<TypePtr> &targs) const {
    auto left = this->left._instantiate(gs, params, targs);
    auto right = this->right._instantiate(gs, params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(gs, left, right);
    }
    return nullptr;
}

TypePtr OrType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    auto left = this->left._instantiate(gs, tc);
    auto right = this->right._instantiate(gs, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(gs, left, right);
    }
    return nullptr;
}

TypePtr OrType::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    auto left = this->left._approximate(gs, tc);
    auto right = this->right._approximate(gs, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(gs, left, right);
    }
    return nullptr;
}

TypePtr AndType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                              const vector<TypePtr> &targs) const {
    auto left = this->left._instantiate(gs, params, targs);
    auto right = this->right._instantiate(gs, params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(gs, left, right);
    }
    return nullptr;
}

TypePtr AndType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    auto left = this->left._instantiate(gs, tc);
    auto right = this->right._instantiate(gs, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(gs, left, right);
    }
    return nullptr;
}

TypePtr AndType::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    auto left = this->left._approximate(gs, tc);
    auto right = this->right._approximate(gs, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(gs, left, right);
    }
    return nullptr;
}

TypePtr AppliedType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) const {
    optional<vector<TypePtr>> newTargs = mungeArgs(this->targs, &TypePtr::_instantiate, gs, params, targs);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr AppliedType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newTargs = mungeArgs(this->targs, &TypePtr::_instantiate, gs, tc);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr AppliedType::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newTargs = mungeArgs(this->targs, &TypePtr::_approximate, gs, tc);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr LambdaParam::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) const {
    ENFORCE(params.size() == targs.size());
    for (auto &el : params) {
        if (el == this->definition) {
            return targs[&el - &params.front()];
        }
    }
    return nullptr;
}

TypePtr Types::replaceSelfType(const GlobalState &gs, const TypePtr &what, const TypePtr &receiver) {
    ENFORCE(what != nullptr);
    auto t = what._replaceSelfType(gs, receiver);
    if (t) {
        return t;
    }
    return what;
}

TypePtr SelfType::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const {
    return receiver;
}

TypePtr OrType::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const {
    auto left = this->left._replaceSelfType(gs, receiver);
    auto right = this->right._replaceSelfType(gs, receiver);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(gs, left, right);
    }
    return nullptr;
}

TypePtr AndType::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const {
    auto left = this->left._replaceSelfType(gs, receiver);
    auto right = this->right._replaceSelfType(gs, receiver);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(gs, left, right);
    }
    return nullptr;
}

} // namespace sorbet::core
