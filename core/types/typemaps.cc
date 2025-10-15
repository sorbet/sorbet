#include "common/common.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

using namespace std;
namespace sorbet::core {

TypePtr Types::instantiateTypeVars(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(tc.isSolved());
    if (tc.isEmpty()) {
        return what;
    }
    ENFORCE(what != nullptr);
    auto t = what._instantiateTypeVars(gs, tc);
    if (t) {
        return t;
    }
    return what;
}

TypePtr Types::approximateTypeVars(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc) {
    ENFORCE(what != nullptr);
    auto t = what._approximateTypeVars(gs, tc, core::Polarity::Positive);
    if (t) {
        return t;
    }
    return what;
}

TypePtr TypeVar::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
    return tc.getInstantiation(sym);
}

TypePtr TypeVar::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
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

optional<vector<TypePtr>> instantiateLambdaParamsInElems(const vector<TypePtr> &elems, const GlobalState &gs,
                                                         absl::Span<const TypeMemberRef> params,
                                                         const vector<TypePtr> &targs, TypePtr selfType) {
    optional<vector<TypePtr>> newElems;
    int i = -1;
    for (auto &e : elems) {
        ++i;
        auto t = e._instantiateLambdaParams(gs, params, targs, selfType);
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

optional<vector<TypePtr>> instantiateTypeVarsInElems(const vector<TypePtr> &elems, const GlobalState &gs,
                                                     const TypeConstraint &tc) {
    optional<vector<TypePtr>> newElems;
    int i = -1;
    for (auto &e : elems) {
        ++i;
        auto t = e._instantiateTypeVars(gs, tc);
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
        auto t = e._approximateTypeVars(gs, tc, polarities[i]);
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

TypePtr TupleType::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                            const vector<TypePtr> &targs, TypePtr selfType) const {
    optional<vector<TypePtr>> newElems = instantiateLambdaParamsInElems(this->elems, gs, params, targs, selfType);
    if (!newElems) {
        return nullptr;
    }
    return make_type<TupleType>(move(*newElems));
}

TypePtr TupleType::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newElems = instantiateTypeVarsInElems(this->elems, gs, tc);
    if (!newElems) {
        return nullptr;
    }
    return make_type<TupleType>(move(*newElems));
}

TypePtr TupleType::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc,
                                        core::Polarity polarity) const {
    PolaritiesStore polarities(this->elems.size(), polarity);
    optional<vector<TypePtr>> newElems = approximateElems(this->elems, gs, tc, polarities);
    if (!newElems) {
        return nullptr;
    }
    return make_type<TupleType>(move(*newElems));
};

TypePtr ShapeType::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                            const vector<TypePtr> &targs, TypePtr selfType) const {
    optional<vector<TypePtr>> newValues = instantiateLambdaParamsInElems(this->values, gs, params, targs, selfType);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(this->keys, move(*newValues));
}

TypePtr ShapeType::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newValues = instantiateTypeVarsInElems(this->values, gs, tc);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(this->keys, move(*newValues));
}

TypePtr ShapeType::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc,
                                        core::Polarity polarity) const {
    PolaritiesStore polarities(this->values.size(), polarity);
    optional<vector<TypePtr>> newValues = approximateElems(this->values, gs, tc, polarities);
    if (!newValues) {
        return nullptr;
    }
    return make_type<ShapeType>(this->keys, move(*newValues));
}

TypePtr OrType::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                         const vector<TypePtr> &targs, TypePtr selfType) const {
    auto left = this->left._instantiateLambdaParams(gs, params, targs, selfType);
    auto right = this->right._instantiateLambdaParams(gs, params, targs, selfType);
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

TypePtr OrType::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
    auto left = this->left._instantiateTypeVars(gs, tc);
    auto right = this->right._instantiateTypeVars(gs, tc);
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

TypePtr OrType::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    auto left = this->left._approximateTypeVars(gs, tc, polarity);
    auto right = this->right._approximateTypeVars(gs, tc, polarity);
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

TypePtr AndType::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                          const vector<TypePtr> &targs, TypePtr selfType) const {
    auto left = this->left._instantiateLambdaParams(gs, params, targs, selfType);
    auto right = this->right._instantiateLambdaParams(gs, params, targs, selfType);
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

TypePtr AndType::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
    auto left = this->left._instantiateTypeVars(gs, tc);
    auto right = this->right._instantiateTypeVars(gs, tc);
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

TypePtr AndType::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const {
    auto left = this->left._approximateTypeVars(gs, tc, polarity);
    auto right = this->right._approximateTypeVars(gs, tc, polarity);
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

TypePtr AppliedType::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                              const vector<TypePtr> &targs, TypePtr selfType) const {
    optional<vector<TypePtr>> newTargs = instantiateLambdaParamsInElems(this->targs, gs, params, targs, selfType);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr AppliedType::_instantiateTypeVars(const GlobalState &gs, const TypeConstraint &tc) const {
    optional<vector<TypePtr>> newTargs = instantiateTypeVarsInElems(this->targs, gs, tc);
    if (!newTargs) {
        return nullptr;
    }
    return make_type<AppliedType>(this->klass, move(*newTargs));
}

TypePtr AppliedType::_approximateTypeVars(const GlobalState &gs, const TypeConstraint &tc,
                                          core::Polarity polarity) const {
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

TypePtr LambdaParam::_instantiateLambdaParams(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                                              const vector<TypePtr> &targs, TypePtr selfType) const {
    ENFORCE(params.size() == targs.size());
    for (auto &el : params) {
        if (el == this->definition) {
            return targs[&el - &params.front()];
        }
    }
    // TODO(jez) I'm starting to think that the "_instantiateLambdaParams" approach with a threaded
    // `selfType` parameter can be made to work, but it should be powered by a `NewSelfType { upperBound = ... }`
    // type, because we're going to need an upperBound.
    //
    // ... but now that I type that out, it's still not quite right, because there was value we were
    // getting out of the LambdaParam/SelfTypeParam distinction--we need to somehow make sure that
    // two T.self_type's in unrelated hierarchies are never compared to each other...
    if (this->definition == core::Symbols::T_SelfType()) {
        return selfType;
    }
    return nullptr;
}

} // namespace sorbet::core
