#include "core/TypeConstraint.h"
#include "common/formatting.h"
#include "core/GlobalState.h"
#include "core/Symbols.h"

using namespace std;

namespace sorbet::core {

bool TypeConstraint::isEmpty() const {
    return upperBounds.empty() && lowerBounds.empty();
}

void TypeConstraint::defineDomain(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &typeParams) {
    // ENFORCE(isEmpty()); // unfortunately this is false. See
    // test/testdata/infer/generic_methods/countraints_crosstalk.rb
    for (const auto &tp : typeParams) {
        ENFORCE(tp.data(gs)->isTypeArgument());
        auto typ = cast_type_const<TypeVar>(tp.data(gs)->resultType);
        ENFORCE(typ != nullptr);

        if (tp.data(gs)->isCovariant()) {
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

    // instatiate types to upper bound approximations
    for (auto &k : upperBounds) {
        auto &tv = k.first;
        auto &bound = k.second;
        if (bound == Types::top()) {
            continue;
        }
        auto approximation = bound->_approximate(gs, *this);
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
        auto approximation = bound->_approximate(gs, *this);
        if (approximation) {
            sol = approximation;
        } else {
            ENFORCE(bound->isFullyDefined());
            sol = bound;
        }
    }

    for (auto &k : upperBounds) {
        auto &tv = k.first;
        auto &upperBound = k.second;
        auto &sol = findSolution(tv);
        if (!sol) {
            sol = upperBound;
        }
        if (upperBound) {
            cantSolve = !Types::isSubType(gs, findSolution(tv), upperBound);
            if (cantSolve) {
                return false;
            }
        }
    }

    for (auto &k : lowerBounds) {
        auto &tv = k.first;
        auto &lowerBound = k.second;

        cantSolve = !Types::isSubType(gs, lowerBound, findSolution(tv));
        if (cantSolve) {
            return false;
        }
    }

    wasSolved = true;
    return true;
}

bool TypeConstraint::rememberIsSubtype(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
    ENFORCE(!wasSolved);
    if (auto t1p = cast_type_const<TypeVar>(t1)) {
        auto &entry = findUpperBound(t1p->sym);
        if (!entry) {
            entry = t2;
        } else if (t2->isFullyDefined()) {
            entry = Types::all(gs, entry, t2);
        } else {
            entry = AndType::make_shared(entry, t2);
        }
    } else {
        auto t2p = cast_type_const<TypeVar>(t2);
        ENFORCE(t2p != nullptr);
        auto &entry = findLowerBound(t2p->sym);
        if (!entry) {
            entry = t1;
        } else if (t1->isFullyDefined()) {
            entry = Types::any(gs, entry, t1);
        } else {
            entry = AndType::make_shared(entry, t1);
        }
    }
    return true;
}

bool TypeConstraint::isAlreadyASubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) const {
    if (auto t1p = cast_type_const<TypeVar>(t1)) {
        if (!hasLowerBound(t1p->sym)) {
            return Types::isSubType(gs, Types::top(), t2);
        }
        return Types::isSubType(gs, findLowerBound(t1p->sym), t2);
    } else {
        auto t2p = cast_type_const<TypeVar>(t2);
        ENFORCE(t2p != nullptr);
        if (!hasUpperBound(t2p->sym)) {
            return Types::isSubType(gs, t1, Types::bottom());
        }
        return Types::isSubType(gs, t1, findUpperBound(t2p->sym));
    }
}

TypePtr TypeConstraint::getInstantiation(SymbolRef sym) const {
    ENFORCE(wasSolved);
    return findSolution(sym);
}

unique_ptr<TypeConstraint> TypeConstraint::deepCopy() const {
    ENFORCE(!wasSolved);
    auto res = make_unique<TypeConstraint>();
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

TypePtr &TypeConstraint::findUpperBound(SymbolRef forWhat) {
    for (auto &entry : this->upperBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    auto &inserted = this->upperBounds.emplace_back();
    inserted.first = forWhat;
    return inserted.second;
}

TypePtr &TypeConstraint::findLowerBound(SymbolRef forWhat) {
    for (auto &entry : this->lowerBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    auto &inserted = this->lowerBounds.emplace_back();
    inserted.first = forWhat;
    return inserted.second;
}

TypePtr &TypeConstraint::findSolution(SymbolRef forWhat) {
    for (auto &entry : this->solution) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    auto &inserted = this->solution.emplace_back();
    inserted.first = forWhat;
    return inserted.second;
}

TypePtr TypeConstraint::findUpperBound(SymbolRef forWhat) const {
    for (auto &entry : this->upperBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    Exception::raise("should never happen");
}

TypePtr TypeConstraint::findLowerBound(SymbolRef forWhat) const {
    for (auto &entry : this->lowerBounds) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    Exception::raise("should never happen");
}

TypePtr TypeConstraint::findSolution(SymbolRef forWhat) const {
    for (auto &entry : this->solution) {
        if (entry.first == forWhat) {
            return entry.second;
        }
    }
    Exception::raise("should never happen");
}

InlinedVector<SymbolRef, 4> TypeConstraint::getDomain() const {
    ENFORCE(isSolved());
    InlinedVector<SymbolRef, 4> ret;
    for (auto &entry : this->solution) {
        ret.emplace_back(entry.first);
    }
    return ret;
}

std::string TypeConstraint::toString(const core::GlobalState &gs) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "upperBounds: [{}]\n",
                   fmt::map_join(
                       this->upperBounds.begin(), this->upperBounds.end(), ", ", [&gs](auto pair) -> auto {
                           return fmt::format("{}: {}", pair.first.toString(gs), pair.second->show(gs));
                       }));
    fmt::format_to(buf, "lowerBounds: [{}]\n",
                   fmt::map_join(
                       this->lowerBounds.begin(), this->lowerBounds.end(), ", ", [&gs](auto pair) -> auto {
                           return fmt::format("{}: {}", pair.first.toString(gs), pair.second->show(gs));
                       }));
    fmt::format_to(buf, "solution: [{}]\n",
                   fmt::map_join(
                       this->solution.begin(), this->solution.end(), ", ", [&gs](auto pair) -> auto {
                           return fmt::format("{}: {}", pair.first.toString(gs), pair.second->show(gs));
                       }));
    return to_string(buf);
}

} // namespace sorbet::core
