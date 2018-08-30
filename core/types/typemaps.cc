#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Hashing.h"
#include "core/Names.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;
namespace sorbet {
namespace core {

shared_ptr<Type> Types::instantiate(Context ctx, const shared_ptr<Type> &what,
                                    const InlinedVector<SymbolRef, 4> &params, const vector<shared_ptr<Type>> &targs) {
    ENFORCE(what.get());
    auto t = what->_instantiate(ctx, params, targs);
    if (t) {
        return t;
    }
    return what;
}

shared_ptr<Type> Types::instantiate(Context ctx, const shared_ptr<Type> &what, const TypeConstraint &tc) {
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

shared_ptr<Type> Types::approximate(Context ctx, const shared_ptr<Type> &what, const TypeConstraint &tc) {
    ENFORCE(what.get());
    auto t = what->_approximate(ctx, tc);
    if (t) {
        return t;
    }
    return what;
}

shared_ptr<Type> TypeVar::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                       const vector<shared_ptr<Type>> &targs) {
    return nullptr;
}

shared_ptr<Type> TypeVar::_instantiate(Context ctx, const TypeConstraint &tc) {
    return tc.getInstantiation(sym);
}

shared_ptr<Type> TypeVar::_approximate(Context ctx, const TypeConstraint &tc) {
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

shared_ptr<Type> ClassType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                         const vector<shared_ptr<Type>> &targs) {
    return nullptr;
}

shared_ptr<Type> LiteralType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                           const vector<shared_ptr<Type>> &targs) {
    return nullptr;
}

shared_ptr<Type> TupleType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                         const vector<shared_ptr<Type>> &targs) {
    bool changed = false;
    vector<shared_ptr<Type>> newElems;
    for (auto &a : this->elems) {
        auto t = a->_instantiate(ctx, params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.push_back(t);
        } else {
            newElems.push_back(nullptr);
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

shared_ptr<Type> TupleType::_instantiate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<shared_ptr<Type>> newElems;
    for (auto &a : this->elems) {
        auto t = a->_instantiate(ctx, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.push_back(t);
        } else {
            newElems.push_back(nullptr);
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

shared_ptr<Type> TupleType::_approximate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<shared_ptr<Type>> newElems;
    for (auto &a : this->elems) {
        auto t = a->_approximate(ctx, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newElems.push_back(t);
        } else {
            newElems.push_back(nullptr);
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

shared_ptr<Type> ShapeType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                         const vector<shared_ptr<Type>> &targs) {
    bool changed = false;
    vector<shared_ptr<Type>> newValues;
    for (auto &a : this->values) {
        auto t = a->_instantiate(ctx, params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.push_back(t);
        } else {
            newValues.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_shared<ShapeType>(Types::hashOfUntyped(), this->keys, newValues);
    }
    return nullptr;
}

shared_ptr<Type> ShapeType::_instantiate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<shared_ptr<Type>> newValues;
    for (auto &a : this->values) {
        auto t = a->_instantiate(ctx, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.push_back(t);
        } else {
            newValues.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_shared<ShapeType>(Types::hashOfUntyped(), this->keys, newValues);
    }
    return nullptr;
}

shared_ptr<Type> ShapeType::_approximate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<shared_ptr<Type>> newValues;
    for (auto &a : this->values) {
        auto t = a->_approximate(ctx, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newValues.push_back(t);
        } else {
            newValues.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newValues[i]) {
            newValues[i] = this->values[i];
            i++;
        }
        return make_shared<ShapeType>(Types::hashOfUntyped(), this->keys, newValues);
    }
    return nullptr;
}

shared_ptr<Type> OrType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                      const vector<shared_ptr<Type>> &targs) {
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

shared_ptr<Type> OrType::_instantiate(Context ctx, const TypeConstraint &tc) {
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

shared_ptr<Type> OrType::_approximate(Context ctx, const TypeConstraint &tc) {
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

shared_ptr<Type> AndType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                       const vector<shared_ptr<Type>> &targs) {
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

shared_ptr<Type> AndType::_instantiate(Context ctx, const TypeConstraint &tc) {
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

shared_ptr<Type> AndType::_approximate(Context ctx, const TypeConstraint &tc) {
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

shared_ptr<Type> AppliedType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                           const vector<shared_ptr<Type>> &targs) {
    bool changed = false;
    vector<shared_ptr<Type>> newTargs;
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_instantiate(ctx, params, targs);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.push_back(t);
        } else {
            newTargs.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_shared<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

shared_ptr<Type> AppliedType::_instantiate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<shared_ptr<Type>> newTargs;
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_instantiate(ctx, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.push_back(t);
        } else {
            newTargs.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_shared<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

shared_ptr<Type> AppliedType::_approximate(Context ctx, const TypeConstraint &tc) {
    bool changed = false;
    vector<shared_ptr<Type>> newTargs;
    // TODO: make it not allocate if returns nullptr
    for (auto &a : this->targs) {
        auto t = a->_approximate(ctx, tc);
        if (changed || t) {
            changed = true;
            if (!t) {
                t = a;
            }
            newTargs.push_back(t);
        } else {
            newTargs.push_back(nullptr);
        }
    }
    if (changed) {
        int i = 0;
        while (!newTargs[i]) {
            newTargs[i] = this->targs[i];
            i++;
        }
        return make_shared<AppliedType>(this->klass, newTargs);
    }

    return nullptr;
}

shared_ptr<Type> SelfTypeParam::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                             const vector<shared_ptr<Type>> &targs) {
    return nullptr;
}

shared_ptr<Type> LambdaParam::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                           const vector<shared_ptr<Type>> &targs) {
    for (auto &el : params) {
        if (el == this->definition) {
            return targs[&el - &params.front()];
        }
    }
    return nullptr;
}

shared_ptr<Type> Type::_approximate(Context ctx, const TypeConstraint &tc) {
    return nullptr;
}

shared_ptr<Type> Type::_instantiate(Context ctx, const TypeConstraint &tc) {
    return nullptr;
}

shared_ptr<Type> SelfType::_instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                        const vector<shared_ptr<Type>> &targs) {
    return nullptr;
}

shared_ptr<Type> Types::replaceSelfType(Context ctx, const shared_ptr<Type> &what, const shared_ptr<Type> &receiver) {
    ENFORCE(what.get());
    auto t = what->_replaceSelfType(ctx, receiver);
    if (t) {
        return t;
    }
    return what;
}

shared_ptr<Type> SelfType::_replaceSelfType(Context ctx, const shared_ptr<Type> &receiver) {
    return receiver;
}

shared_ptr<Type> Type::_replaceSelfType(Context ctx, const shared_ptr<Type> &receiver) {
    return nullptr;
}

shared_ptr<Type> OrType::_replaceSelfType(Context ctx, const shared_ptr<Type> &receiver) {
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

shared_ptr<Type> AndType::_replaceSelfType(Context ctx, const shared_ptr<Type> &receiver) {
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
} // namespace core
} // namespace sorbet