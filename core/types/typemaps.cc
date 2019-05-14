#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Hashing.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;
namespace sorbet::core {

TypePtr Types::instantiate(Context ctx, const TypePtr &what, const InlinedVector<SymbolRef, 4> &params,
                           const vector<TypePtr> &targs) {
    ENFORCE(what.get());
    auto t = what->_instantiate(ctx, params, targs);
    if (t) {
        return t;
    }
    return what;
}

TypePtr Types::instantiate(Context ctx, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(tc.isSolved());
    if (tc.isEmpty()) {
        return what;
    }
    ENFORCE(what.get());
    auto t = what->_instantiate(ctx, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr Types::approximate(Context ctx, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(what.get());
    auto t = what->_approximate(ctx, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr TypeVar::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr TypeVar::_instantiate(Context ctx, const TypeConstraint &tc) {
    return tc.getInstantiation(sym);
}

TypePtr TypeVar::_approximate(Context ctx, const TypeConstraint &tc) {
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

TypePtr ClassType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr LiteralType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr TupleType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    bool changed = false;
    vector<TypePtr> newElems;
    newElems.reserve(this->elems.size());
    for (auto &a : this->elems) {
        auto t = a->_instantiate(ctx, params, targs);
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
        return TupleType::build(ctx, newElems);
    }
    return nullptr;
}

TypePtr TupleType::_instantiate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newElems;
    newElems.reserve(this->elems.size());
    for (auto &a : this->elems) {
        auto t = a->_instantiate(ctx, tc);
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
        return TupleType::build(ctx, newElems);
    }
    return nullptr;
}

TypePtr TupleType::_approximate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newElems;
    newElems.reserve(this->elems.size());
    for (auto &a : this->elems) {
        auto t = a->_approximate(ctx, tc);
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
        return TupleType::build(ctx, newElems);
    }
    return nullptr;
};

TypePtr ShapeType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    bool changed = false;
    vector<TypePtr> newValues;
    newValues.reserve(this->values.size());
    for (auto &a : this->values) {
        auto t = a->_instantiate(ctx, params, targs);
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
        return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, newValues);
    }
    return nullptr;
}

TypePtr ShapeType::_instantiate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newValues;
    newValues.reserve(this->values.size());
    for (auto &a : this->values) {
        auto t = a->_instantiate(ctx, tc);
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
        return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, newValues);
    }
    return nullptr;
}

TypePtr ShapeType::_approximate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newValues;
    newValues.reserve(this->values.size());
    for (auto &a : this->values) {
        auto t = a->_approximate(ctx, tc);
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
        return make_type<ShapeType>(Types::hashOfUntyped(), this->keys, newValues);
    }
    return nullptr;
}

TypePtr OrType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    auto left = this->left->_instantiate(ctx, params, targs);
    auto right = this->right->_instantiate(ctx, params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(ctx, left, right);
    }
    return nullptr;
}

TypePtr OrType::_instantiate(Context ctx, const TypeConstraint &tc) {
    auto left = this->left->_instantiate(ctx, tc);
    auto right = this->right->_instantiate(ctx, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(ctx, left, right);
    }
    return nullptr;
}

TypePtr OrType::_approximate(Context ctx, const TypeConstraint &tc) {
    auto left = this->left->_approximate(ctx, tc);
    auto right = this->right->_approximate(ctx, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(ctx, left, right);
    }
    return nullptr;
}

TypePtr AndType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    auto left = this->left->_instantiate(ctx, params, targs);
    auto right = this->right->_instantiate(ctx, params, targs);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(ctx, left, right);
    }
    return nullptr;
}

TypePtr AndType::_instantiate(Context ctx, const TypeConstraint &tc) {
    auto left = this->left->_instantiate(ctx, tc);
    auto right = this->right->_instantiate(ctx, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(ctx, left, right);
    }
    return nullptr;
}

TypePtr AndType::_approximate(Context ctx, const TypeConstraint &tc) {
    auto left = this->left->_approximate(ctx, tc);
    auto right = this->right->_approximate(ctx, tc);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(ctx, left, right);
    }
    return nullptr;
}

TypePtr AppliedType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) {
    bool changed = false;
    vector<TypePtr> newTargs;
    newTargs.reserve(this->targs.size());
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_instantiate(ctx, params, targs);
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
        return make_type<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

TypePtr AppliedType::_instantiate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newTargs;
    newTargs.reserve(this->targs.size());
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_instantiate(ctx, tc);
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
        return make_type<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

TypePtr AppliedType::_approximate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<TypePtr> newTargs;
    newTargs.reserve(this->targs.size());
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_approximate(ctx, tc);
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
        return make_type<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

TypePtr SelfTypeParam::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                    const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr LambdaParam::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                  const vector<TypePtr> &targs) {
    ENFORCE(params.size() == targs.size());
    for (auto &el : params) {
        if (el == this->definition) {
            return targs[&el - &params.front()];
        }
    }
    return nullptr;
}

TypePtr Type::_approximate(Context ctx, const TypeConstraint &tc) {
    return nullptr;
}

TypePtr Type::_instantiate(Context ctx, const TypeConstraint &tc) {
    return nullptr;
}

TypePtr SelfType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params, const vector<TypePtr> &targs) {
    return nullptr;
}

TypePtr Types::replaceSelfType(Context ctx, const TypePtr &what, const TypePtr &receiver) {
    ENFORCE(what.get());
    auto t = what->_replaceSelfType(ctx, receiver);
    if (t) {
        return t;
    }
    return what;
}

TypePtr SelfType::_replaceSelfType(Context ctx, const TypePtr &receiver) {
    return receiver;
}

TypePtr Type::_replaceSelfType(Context ctx, const TypePtr &receiver) {
    return nullptr;
}

TypePtr OrType::_replaceSelfType(Context ctx, const TypePtr &receiver) {
    auto left = this->left->_replaceSelfType(ctx, receiver);
    auto right = this->right->_replaceSelfType(ctx, receiver);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::any(ctx, left, right);
    }
    return nullptr;
}

TypePtr AndType::_replaceSelfType(Context ctx, const TypePtr &receiver) {
    auto left = this->left->_replaceSelfType(ctx, receiver);
    auto right = this->right->_replaceSelfType(ctx, receiver);
    if (left || right) {
        if (!left) {
            left = this->left;
        }
        if (!right) {
            right = this->right;
        }
        return Types::all(ctx, left, right);
    }
    return nullptr;
}

unsigned int Type::hash(const GlobalState &gs) const {
    return _hash(this->toString(gs)); // TODO: make something better
}

} // namespace sorbet::core
