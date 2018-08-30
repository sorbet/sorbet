#include "core/TypeConstraint.h"
#include "core/Symbols.h"
using namespace std;
namespace sorbet {
namespace core {

bool TypeConstraint::isEmpty() const {
    return upperBounds.empty() && lowerBounds.empty();
}

void TypeConstraint::defineDomain(Context ctx, const InlinedVector<SymbolRef, 4> &typeParams) {
    // ENFORCE(isEmpty()); // unfortunately this is false. See
    // test/testdata/infer/generic_methods/countraints_crosstalk.rb
    for (const auto &tp : typeParams) {
        ENFORCE(tp.data(ctx).isTypeArgument());
        auto typ = cast_type<TypeVar>(tp.data(ctx).resultType.get());
        ENFORCE(typ != nullptr);

        if (tp.data(ctx).isCovariant()) {
            findLowerBound(typ->sym);
        } else {
            findUpperBound(typ->sym);
        }
    }
}
bool TypeConstraint::solve(Context ctx) {
    if (cantSolve) {
        return false;
    }
    if (wasSolved) {
        return true;
    }

    // instatiate types to upper bound approximations
    for (auto &k : upperBounds) {
        auto &tv = k.first;
        auto &bound = k.second;
        if (!bound) {
            continue;
        }
        auto approximation = bound->_approximate(ctx, *this);
        if (approximation) {
            findSolution(tv) = approximation;
        } else {
            ENFORCE(bound->isFullyDefined());
            findSolution(tv) = bound;
        }
    }
    // or lower bound approximation, if there is no upper bound
    for (auto &k : lowerBounds) {
        auto &tv = k.first;
        auto &bound = k.second;
        auto &sol = findSolution(tv);
        if (sol) {
            continue;
        }
        if (!bound) {
            bound = Types::bottom();
        }
        auto approximation = bound->_approximate(ctx, *this);
        if (approximation) {
            sol = approximation;
        } else {
            ENFORCE(bound->isFullyDefined());
            sol = bound;
        }
    }

    for (auto &k : lowerBounds) {
        auto &tv = k.first;
        auto &lowerBound = k.second;

        cantSolve = !Types::isSubType(ctx, lowerBound, findSolution(tv));
        if (cantSolve) {
            return false;
        }
    }
    for (auto &k : upperBounds) {
        auto &tv = k.first;
        auto &upperBound = k.second;

        cantSolve = !Types::isSubType(ctx, findSolution(tv), upperBound);
        if (cantSolve) {
            return false;
        }
    }
    wasSolved = true;
    return true;
}

bool TypeConstraint::rememberIsSubtype(Context ctx, const shared_ptr<Type> &t1, const shared_ptr<Type> &t2) {
    ENFORCE(!wasSolved);
    if (auto t1p = cast_type<TypeVar>(t1.get())) {
        auto &entry = findUpperBound(t1p->sym);
        if (!entry) {
            entry = t2;
        } else if (t2->isFullyDefined()) {
            entry = Types::all(ctx, entry, t2);
        } else {
            entry = AndType::make_shared(entry, t2);
        }
    } else {
        auto t2p = cast_type<TypeVar>(t2.get());
        ENFORCE(t2p != nullptr);
        auto &entry = findLowerBound(t2p->sym);
        if (!entry) {
            entry = t1;
        } else if (t1->isFullyDefined()) {
            entry = Types::any(ctx, entry, t1);
        } else {
            entry = AndType::make_shared(entry, t1);
        }
    }
    return true;
}

bool TypeConstraint::isAlreadyASubType(Context ctx, const shared_ptr<Type> &t1, const shared_ptr<Type> &t2) const {
    if (auto t1p = cast_type<TypeVar>(t1.get())) {
        if (!hasLowerBound(t1p->sym)) {
            return Types::isSubType(ctx, Types::top(), t2);
        }
        return Types::isSubType(ctx, findLowerBound(t1p->sym), t2);
    } else {
        auto t2p = cast_type<TypeVar>(t2.get());
        ENFORCE(t2p != nullptr);
        if (!hasUpperBound(t2p->sym)) {
            return Types::isSubType(ctx, t1, Types::bottom());
        }
        return Types::isSubType(ctx, t1, findUpperBound(t2p->sym));
    }
}

shared_ptr<Type> TypeConstraint::getInstantiation(SymbolRef sym) const {
    ENFORCE(wasSolved);
    return findSolution(sym);
}

shared_ptr<TypeConstraint> TypeConstraint::deepCopy() const {
    ENFORCE(!wasSolved);
    auto res = make_shared<TypeConstraint>();
    res->lowerBounds = this->lowerBounds;
    res->upperBounds = this->upperBounds;
    return res;
}
TypeConstraint TypeConstraint::makeEmptyFrozenConstraint() {
    TypeConstraint res;
    res.wasSolved = true;
    return res;
}

TypeConstraint TypeConstraint::EmptyFrozenConstraint(makeEmptyFrozenConstraint());

bool TypeConstraint::hasUpperBound(SymbolRef forWhat) const {
    for (auto &entry : this->upperBounds) {
        if (entry.first == forWhat) {
            return true;
        }
    }
    return false;
}

bool TypeConstraint::hasLowerBound(SymbolRef forWhat) const {
    for (auto &entry : this->lowerBounds) {
        if (entry.first == forWhat) {
            return true;
        }
    }
    return false;
}

shared_ptr<Type> &TypeConstraint::findUpperBound(SymbolRef forWhat) {
    for (auto &entry : this->upperBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    this->upperBounds.emplace_back();
    this->upperBounds.back().first = forWhat;
    return this->upperBounds.back().second;
}

shared_ptr<Type> &TypeConstraint::findLowerBound(SymbolRef forWhat) {
    for (auto &entry : this->lowerBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    this->lowerBounds.emplace_back();
    this->lowerBounds.back().first = forWhat;
    return this->lowerBounds.back().second;
}

shared_ptr<Type> &TypeConstraint::findSolution(SymbolRef forWhat) {
    for (auto &entry : this->solution) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    this->solution.emplace_back();
    this->solution.back().first = forWhat;
    return this->solution.back().second;
}

shared_ptr<Type> TypeConstraint::findUpperBound(SymbolRef forWhat) const {
    for (auto &entry : this->upperBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    Error::raise("should never happen");
}

shared_ptr<Type> TypeConstraint::findLowerBound(SymbolRef forWhat) const {
    for (auto &entry : this->lowerBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    Error::raise("should never happen");
}

shared_ptr<Type> TypeConstraint::findSolution(SymbolRef forWhat) const {
    for (auto &entry : this->solution) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    Error::raise("should never happen");
}

} // namespace core
} // namespace sorbet
