#include "absl/base/casts.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;
namespace sorbet::core {

TypePtr Types::instantiate(const GlobalState &gs, const TypePtr &what, absl::Span<const TypeMemberRef> params,
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
    auto t = what._approximate(gs, tc, core::Polarity::Positive);
    if (t) {
        return t;
    }
    return what;
}

TypePtr TypeVar::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    return tc.getInstantiation(sym);
}

TypePtr TypeVar::_approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    switch (polarity) {
        case core::Polarity::Positive:
        case core::Polarity::Neutral: {
            if (tc.hasUpperBound(sym)) {
                auto bound = tc.findUpperBound(sym);
                if (bound.isFullyDefined()) {
                    return bound;
                }
            } else if (tc.hasLowerBound(sym)) {
                auto bound = tc.findLowerBound(sym);
                if (bound.isFullyDefined() && !bound.isBottom()) {
                    return bound;
                }
            }
            break;
        }
        case core::Polarity::Negative: {
            if (tc.hasLowerBound(sym)) {
                auto bound = tc.findLowerBound(sym);
                if (bound.isFullyDefined()) {
                    return bound;
                }
            } else if (tc.hasUpperBound(sym)) {
                auto bound = tc.findUpperBound(sym);
                if (bound.isFullyDefined()) {
                    return bound;
                }
            }
            break;
        }
    }
    // TODO: in many languages this method is a huge adhoc heuristic
    // let's see if we can keep it small
    return Types::top();
}

namespace {

template <typename... MethodArgs>
optional<vector<TypePtr>> instantiateElems(const vector<TypePtr> &elems, const MethodArgs &...methodArgs) {
    optional<vector<TypePtr>> newElems;
    int i = -1;
    for (auto &e : elems) {
        ++i;
        auto t = e._instantiate(methodArgs...);
        if (!newElems.has_value() && !t) {
            continue;
        }

        if (!newElems.has_value()) {
            // Oops, need to fixup all the elements that should be there.
            newElems.emplace();
            newElems->reserve(elems.size());
            for (int j = 0; j < i; ++j) {
                newElems->emplace_back(elems[j]);
            }
        }

        if (!t) {
            t = e;
        }

        ENFORCE(newElems->size() == i);
        newElems->emplace_back(move(t));
    }
    return newElems;
}

// Matches the 4 used in the vector backing ClassOrModuleRef::typeMembers()
using PolaritiesStore = InlinedVector<core::Polarity, 4>;

optional<vector<TypePtr>> approximateElems(const vector<TypePtr> &elems, const GlobalState &gs,
                                           const TypeConstraint &tc, PolaritiesStore &polarities) {
    optional<vector<TypePtr>> newElems;
    int i = -1;
    for (auto &e : elems) {
        ++i;
        auto t = e._approximate(gs, tc, polarities[i]);
        if (!newElems.has_value() && !t) {
            continue;
        }

        if (!newElems.has_value()) {
            // Oops, need to fixup all the elements that should be there.
            newElems.emplace();
            newElems->reserve(elems.size());
            for (int j = 0; j < i; ++j) {
                newElems->emplace_back(elems[j]);
            }
        }

        if (!t) {
            t = e;
        }

        ENFORCE(newElems->size() == i);
        newElems->emplace_back(move(t));
    }
    return newElems;
}

} // anonymous namespace

TypePtr TupleType::_instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                const vector<TypePtr> &targs) const {
    optional<vector<TypePtr>> newElems = instantiateElems(this->elems, gs, params, targs);
    if (!newElems) {
        return nullptr;
    }
    return make_type<TupleType>(move(*newElems));
}

TypePtr TupleType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newElems = instantiateElems(this->elems, gs, tc);
    if (!newElems) {
        return nullptr;
    }
    return make_type<TupleType>(move(*newElems));
}

TypePtr TupleType::_approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    PolaritiesStore polarities(this->elems.size(), polarity);
    optional<vector<TypePtr>> newElems = approximateElems(this->elems, gs, tc, polarities);
    if (!newElems) {
        return nullptr;
    }
    return make_type<TupleType>(move(*newElems));
};

TypePtr ShapeType::_instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                const vector<TypePtr> &targs) const {
    optional<vector<TypePtr>> newValues = instantiateElems(this->values, gs, params, targs);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(this->keys, move(*newValues));
}

TypePtr ShapeType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newValues = instantiateElems(this->values, gs, tc);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(this->keys, move(*newValues));
}

TypePtr ShapeType::_approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    PolaritiesStore polarities(this->values.size(), polarity);
    optional<vector<TypePtr>> newValues = approximateElems(this->values, gs, tc, polarities);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(this->keys, move(*newValues));
}

TypePtr OrType::_instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
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

TypePtr OrType::_approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    auto left = this->left._approximate(gs, tc, polarity);
    auto right = this->right._approximate(gs, tc, polarity);
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

TypePtr AndType::_instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
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

TypePtr AndType::_approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    auto left = this->left._approximate(gs, tc, polarity);
    auto right = this->right._approximate(gs, tc, polarity);
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

TypePtr AppliedType::_instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                  const vector<TypePtr> &targs) const {
    optional<vector<TypePtr>> newTargs = instantiateElems(this->targs, gs, params, targs);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr AppliedType::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newTargs = instantiateElems(this->targs, gs, tc);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr AppliedType::_approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    PolaritiesStore polarities;
    for (auto typeMember : this->klass.data(gs)->typeMembers()) {
        switch (typeMember.data(gs)->variance()) {
            case core::Variance::ContraVariant:
                polarities.emplace_back(core::Polarities::negatePolarity(polarity));
                break;
            case core::Variance::Invariant:
            case core::Variance::CoVariant:
                polarities.emplace_back(polarity);
                break;
        }
    }

    optional<vector<TypePtr>> newTargs = approximateElems(this->targs, gs, tc, polarities);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr LambdaParam::_instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
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
