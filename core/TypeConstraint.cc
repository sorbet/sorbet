#include "core/TypeConstraint.h"
#include "common/strings/formatting.h"
#include "core/GlobalState.h"
#include "core/Symbols.h"
#include <utility>

using namespace std;

namespace sorbet::core {

bool TypeConstraint::isEmpty() const {
    return this->bounds.empty();
}

void TypeConstraint::defineDomain(const GlobalState &gs, absl::Span<const TypeArgumentRef> typeParams) {
    // ENFORCE(isEmpty()); // unfortunately this is false. See
    // test/testdata/infer/generic_methods/countraints_crosstalk.rb
    for (const auto &ta : typeParams) {
        auto typ = cast_type<TypeVar>(ta.data(gs)->resultType);
        ENFORCE(typ != nullptr);

        if (ta.data(gs)->flags.isCovariant) {
            findLowerBound(typ->sym) = Types::bottom();
        } else {
            findUpperBound(typ->sym) = Types::top();
        }
    }
}
bool TypeConstraint::solve(const GlobalState &gs) {
    if (cantSolve) {
        return false;
    }
    if (wasSolved) {
        return true;
    }

    for (auto &bound : this->bounds) {
        ENFORCE(bound.lower.isFullyDefined());
        ENFORCE(bound.upper.isFullyDefined());
        cantSolve = !Types::isSubType(gs, bound.lower, bound.upper);
        if (cantSolve) {
            return false;
        }
    }

    wasSolved = true;
    return true;
    //
    // // instantiate types to upper bound approximations
    // for (auto &k : upperBounds) {
    //     auto &tv = k.first;
    //     auto &bound = k.second;
    //     if (bound == Types::top()) {
    //         continue;
    //     }
    //     auto approximation = bound._approximate(gs, *this, core::Polarity::Positive);
    //     if (approximation) {
    //         findSolution(tv) = approximation;
    //     } else {
    //         ENFORCE(bound.isFullyDefined());
    //         findSolution(tv) = bound;
    //     }
    // }
    // // or lower bound approximation, if there is no upper bound
    // for (auto &k : lowerBounds) {
    //     auto &tv = k.first;
    //     auto &bound = k.second;
    //     auto &sol = findSolution(tv);
    //     if (sol) {
    //         continue;
    //     }
    //     auto approximation = bound._approximate(gs, *this, core::Polarity::Positive);
    //     if (approximation) {
    //         sol = approximation;
    //     } else {
    //         ENFORCE(bound.isFullyDefined());
    //         sol = bound;
    //     }
    // }
    //
    // for (auto &k : upperBounds) {
    //     auto &tv = k.first;
    //     auto &upperBound = k.second;
    //     auto &sol = findSolution(tv);
    //     if (!sol) {
    //         sol = upperBound;
    //     }
    //     if (upperBound) {
    //         cantSolve = !Types::isSubType(gs, findSolution(tv), upperBound);
    //         if (cantSolve) {
    //             return false;
    //         }
    //     }
    // }
    //
    // for (auto &k : lowerBounds) {
    //     auto &tv = k.first;
    //     auto &lowerBound = k.second;
    //
    //     cantSolve = !Types::isSubType(gs, lowerBound, findSolution(tv));
    //     if (cantSolve) {
    //         return false;
    //     }
    // }
    //
    // wasSolved = true;
    // return true;
}

bool TypeConstraint::rememberIsSubtype(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    ENFORCE(!wasSolved);
    if (auto t1p = cast_type<TypeVar>(t1)) {
        auto &entry = findUpperBound(t1p->sym);
        if (!entry) {
            entry = t2;
        } else if (t2.isFullyDefined()) {
            entry = Types::all(gs, entry, t2);
        } else {
            entry = AndType::make_shared(entry, t2);
        }
    } else {
        auto t2p = cast_type<TypeVar>(t2);
        ENFORCE(t2p != nullptr);
        auto &entry = findLowerBound(t2p->sym);
        if (!entry) {
            entry = t1;
        } else if (t1.isFullyDefined()) {
            entry = Types::any(gs, entry, t1);
        } else {
            entry = AndType::make_shared(entry, t1);
        }
    }
    return true;
}

bool TypeConstraint::isAlreadyASubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) const {
    if (auto t1p = cast_type<TypeVar>(t1)) {
        if (!hasLowerBound(t1p->sym)) {
            return Types::isSubType(gs, Types::top(), t2);
        }
        return Types::isSubType(gs, findLowerBound(t1p->sym), t2);
    } else {
        auto t2p = cast_type<TypeVar>(t2);
        ENFORCE(t2p != nullptr);
        if (!hasUpperBound(t2p->sym)) {
            return Types::isSubType(gs, t1, Types::bottom());
        }
        return Types::isSubType(gs, t1, findUpperBound(t2p->sym));
    }
}

const TypeConstraint::Bounds &TypeConstraint::getInstantiation(TypeArgumentRef sym) const {
    ENFORCE(wasSolved);
    return findSolution(sym);
}

unique_ptr<TypeConstraint> TypeConstraint::deepCopy() const {
    auto res = make_unique<TypeConstraint>();
    res->bounds = this->bounds;
    return res;
}
TypeConstraint TypeConstraint::makeEmptyFrozenConstraint() {
    TypeConstraint res;
    res.wasSolved = true;
    return res;
}

TypeConstraint TypeConstraint::EmptyFrozenConstraint(makeEmptyFrozenConstraint());

bool TypeConstraint::hasUpperBound(TypeArgumentRef forWhat) const {
    return absl::c_any_of(this->bounds, [forWhat](auto &entry) { return entry.arg == forWhat; });
}

bool TypeConstraint::hasLowerBound(TypeArgumentRef forWhat) const {
    return absl::c_any_of(this->bounds, [forWhat](auto &entry) { return entry.arg == forWhat; });
}

TypePtr &TypeConstraint::findUpperBound(TypeArgumentRef forWhat) {
    return this->findSolution(forWhat).upper;
}

TypePtr &TypeConstraint::findLowerBound(TypeArgumentRef forWhat) {
    return this->findSolution(forWhat).lower;
}

TypeConstraint::Bounds &TypeConstraint::findSolution(TypeArgumentRef forWhat) {
    for (auto &entry : this->bounds) {
        if (entry.arg == forWhat) {
            return entry;
        }
    }
    auto &inserted = this->bounds.emplace_back(Bounds{forWhat, Types::bottom(), Types::top()});
    return inserted;
}

TypePtr TypeConstraint::findUpperBound(TypeArgumentRef forWhat) const {
    return this->findSolution(forWhat).upper;
}

TypePtr TypeConstraint::findLowerBound(TypeArgumentRef forWhat) const {
    return this->findSolution(forWhat).lower;
}

const TypeConstraint::Bounds &TypeConstraint::findSolution(TypeArgumentRef forWhat) const {
    for (auto &entry : this->bounds) {
        if (entry.arg == forWhat) {
            return entry;
        }
    }
    Exception::raise("Failed to find entry in TypeConstraint::solution for type argument");
}

InlinedVector<SymbolRef, 4> TypeConstraint::getDomain() const {
    ENFORCE(isSolved());
    InlinedVector<SymbolRef, 4> ret;
    for (auto &entry : this->bounds) {
        ret.emplace_back(entry.arg);
    }
    return ret;
}

UnorderedMap<TypeArgumentRef, std::pair<TypePtr, TypePtr>> TypeConstraint::collateBounds(const GlobalState &gs) const {
    auto collated = UnorderedMap<TypeArgumentRef, pair<TypePtr, TypePtr>>{};

    for (const auto &bound : this->bounds) {
        auto [_it, inserted] = collated.insert(std::make_pair(bound.arg, std::make_pair(bound.lower, bound.upper)));
        ENFORCE(inserted, "{} in bounds twice?", bound.arg.show(gs));
    }

    return collated;
}

string TypeConstraint::toString(const core::GlobalState &gs) const {
    auto collated = this->collateBounds(gs);

    return fmt::format("bounds: [{}]\n",
                       fmt::map_join(collated.begin(), collated.end(), ", ", [&gs](auto entry) -> auto {
                           const auto &[sym, bounds] = entry;
                           const auto &[lowerBound, upperBound] = bounds;
                           auto lower = lowerBound != nullptr ? lowerBound.show(gs) : "_";
                           auto upper = upperBound != nullptr ? upperBound.show(gs) : "_";
                           return fmt::format("{} <: {} <: {}", lower, sym.show(gs), upper);
                       }));
}

ErrorSection TypeConstraint::explain(const core::GlobalState &gs) const {
    auto collated = this->collateBounds(gs);
    auto result = vector<ErrorLine>{};

    for (const auto &[sym, bounds] : collated) {
        const auto &[lowerBound, upperBound] = bounds;
        auto typeVar = make_type<TypeVar>(sym).show(gs);
        if (lowerBound == nullptr && upperBound == nullptr) {
            result.emplace_back(ErrorLine::fromWithoutLoc("`{}` is not constrained", typeVar));
        } else if (lowerBound == nullptr) {
            result.emplace_back(
                ErrorLine::fromWithoutLoc("`{}` must be a subtype of `{}`", typeVar, upperBound.show(gs)));
        } else if (upperBound == nullptr) {
            result.emplace_back(
                ErrorLine::fromWithoutLoc("`{}` must be a subtype of `{}`", lowerBound.show(gs), typeVar));
        } else {
            result.emplace_back(
                ErrorLine::fromWithoutLoc("`{}` must be a subtype of `{}` which must be a subtype of `{}`",
                                          lowerBound.show(gs), typeVar, upperBound.show(gs)));
        }
    }

    return ErrorSection("Found no solution for these constraints:", result);
}

} // namespace sorbet::core
