#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Hashing.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;
namespace sorbet::core {

TypePtr Types::instantiate(const GlobalState &gs, const TypePtr &what, const InlinedVector<SymbolRef, 4> &params,
                           const vector<TypePtr> &targs) {
    ENFORCE(what.get());
    auto t = what->_instantiate(gs, params, targs);
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
    ENFORCE(what.get());
    auto t = what->_instantiate(gs, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr Types::approximate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(what.get());
    auto t = what->_approximate(gs, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr TypeVar::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                              const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr TypeVar::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    return tc.getInstantiation(sym);
}

TypePtr TypeVar::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    if (tc.hasUpperBound(sym)) {
        auto bound = tc.findUpperBound(sym);
        if (bound->isFullyDefined()) {
            return bound;
        }
    }
    // TODO: in many languages this method is a huge adhoc heuristic
    // let's see if we can keep it small
    return Types::top();
}

TypePtr ClassType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr LiteralType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr TupleType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                const vector<TypePtr> &targs) {
    bool changed = false;
    vector<TypePtr> newElems;
    newElems.reserve(this->elems.size());
    for (auto &a : this->elems) {
        auto t = a->_instantiate(gs, params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.emplace_back(t);
        } else {
            newElems.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newElems[i]) {
            newElems[i] = this->elems[i];
            i++;
        }
        return TupleType::build(gs, newElems);
    }
    return nullptr;
}

TypePtr TupleType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newElems;
    newElems.reserve(this->elems.size());
    for (auto &a : this->elems) {
        auto t = a->_instantiate(gs, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.emplace_back(t);
        } else {
            newElems.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newElems[i]) {
            newElems[i] = this->elems[i];
            i++;
        }
        return TupleType::build(gs, newElems);
    }
    return nullptr;
}

TypePtr TupleType::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newElems;
    newElems.reserve(this->elems.size());
    for (auto &a : this->elems) {
        auto t = a->_approximate(gs, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.emplace_back(t);
        } else {
            newElems.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newElems[i]) {
            newElems[i] = this->elems[i];
            i++;
        }
        return TupleType::build(gs, newElems);
    }
    return nullptr;
};

TypePtr ShapeType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                const vector<TypePtr> &targs) {
    bool changed = false;
    vector<TypePtr> newValues;
    newValues.reserve(this->values.size());
    for (auto &a : this->values) {
        auto t = a->_instantiate(gs, params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.emplace_back(t);
        } else {
            newValues.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, move(newValues));
    }
    return nullptr;
}

TypePtr ShapeType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newValues;
    newValues.reserve(this->values.size());
    for (auto &a : this->values) {
        auto t = a->_instantiate(gs, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.emplace_back(t);
        } else {
            newValues.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, move(newValues));
    }
    return nullptr;
}

TypePtr ShapeType::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newValues;
    newValues.reserve(this->values.size());
    for (auto &a : this->values) {
        auto t = a->_approximate(gs, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.emplace_back(t);
        } else {
            newValues.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, move(newValues));
    }
    return nullptr;
}

TypePtr OrType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                             const vector<TypePtr> &targs) {
    auto left = this->left->_instantiate(gs, params, targs);
    auto right = this->right->_instantiate(gs, params, targs);
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

TypePtr OrType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    auto left = this->left->_instantiate(gs, tc);
    auto right = this->right->_instantiate(gs, tc);
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

TypePtr OrType::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    auto left = this->left->_approximate(gs, tc);
    auto right = this->right->_approximate(gs, tc);
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
                              const vector<TypePtr> &targs) {
    auto left = this->left->_instantiate(gs, params, targs);
    auto right = this->right->_instantiate(gs, params, targs);
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

TypePtr AndType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    auto left = this->left->_instantiate(gs, tc);
    auto right = this->right->_instantiate(gs, tc);
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

TypePtr AndType::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    auto left = this->left->_approximate(gs, tc);
    auto right = this->right->_approximate(gs, tc);
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
                                  const vector<TypePtr> &targs) {
    bool changed = false;
    vector<TypePtr> newTargs;
    newTargs.reserve(this->targs.size());
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_instantiate(gs, params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.emplace_back(t);
        } else {
            newTargs.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_type<AppliedType>(this->klass, move(newTargs));
    }

    return nullptr;
}

TypePtr AppliedType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newTargs;
    newTargs.reserve(this->targs.size());
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_instantiate(gs, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.emplace_back(t);
        } else {
            newTargs.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_type<AppliedType>(this->klass, move(newTargs));
    }

    return nullptr;
}

TypePtr AppliedType::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newTargs;
    newTargs.reserve(this->targs.size());
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_approximate(gs, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.emplace_back(t);
        } else {
            newTargs.emplace_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_type<AppliedType>(this->klass, move(newTargs));
    }

    return nullptr;
}

TypePtr SelfTypeParam::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                    const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr LambdaParam::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) {
    ENFORCE(params.size() == targs.size());
    for (auto &el : params) {
        if (el == this->definition) {
            return targs[&el - &params.front()];
        }
    }
    return nullptr;
}

TypePtr Type::_approximate(const GlobalState &gs, const TypeConstraint &tc) {
    return nullptr;
}

TypePtr Type::_instantiate(const GlobalState &gs, const TypeConstraint &tc) {
    return nullptr;
}

TypePtr SelfType::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                               const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr Types::replaceSelfType(const GlobalState &gs, const TypePtr &what, const TypePtr &receiver) {
    ENFORCE(what.get());
    auto t = what->_replaceSelfType(gs, receiver);
    if (t) {
        return t;
    }
    return what;
}

TypePtr SelfType::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) {
    return receiver;
}

TypePtr Type::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) {
    return nullptr;
}

TypePtr OrType::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) {
    auto left = this->left->_replaceSelfType(gs, receiver);
    auto right = this->right->_replaceSelfType(gs, receiver);
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

TypePtr AndType::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) {
    auto left = this->left->_replaceSelfType(gs, receiver);
    auto right = this->right->_replaceSelfType(gs, receiver);
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

unsigned int Type::hash(const GlobalState &gs) const {
    return _hash(this->toString(gs)); // TODO: make something better
}

} // namespace sorbet::core
