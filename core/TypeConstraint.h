#ifndef SORBET_TYPECONSTRAINT_H
#define SORBET_TYPECONSTRAINT_H

#include "core/Context.h"
#include "core/SymbolRef.h"
#include "core/TypePtr.h"
namespace sorbet::core {

class TypeConstraint {
    static TypeConstraint makeEmptyFrozenConstraint();
    std::vector<std::pair<SymbolRef, TypePtr>> upperBounds;
    std::vector<std::pair<SymbolRef, TypePtr>> lowerBounds;
    std::vector<std::pair<SymbolRef, TypePtr>> solution;
    bool wasSolved = false;
    bool cantSolve = false;
    TypePtr &findUpperBound(SymbolRef forWhat);
    TypePtr &findLowerBound(SymbolRef forWhat);
    TypePtr &findSolution(SymbolRef forWhat);

    UnorderedMap<SymbolRef, std::pair<TypePtr, TypePtr>> collateBounds(const GlobalState &gs) const;

public:
    TypeConstraint() = default;
    TypeConstraint(const TypeConstraint &) = delete;
    TypeConstraint(TypeConstraint &&) = default;
    void defineDomain(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &typeParams);
    bool hasUpperBound(SymbolRef forWhat) const;
    bool hasLowerBound(SymbolRef forWhat) const;
    TypePtr findSolution(SymbolRef forWhat) const;
    TypePtr findUpperBound(SymbolRef forWhat) const;
    TypePtr findLowerBound(SymbolRef forWhat) const;

    bool isEmpty() const;
    inline bool isSolved() const {
        return wasSolved;
    }

    // At least one of arguments has to be a typevar
    bool rememberIsSubtype(const GlobalState &gs, const TypePtr &, const TypePtr &);

    // At least one of arguments has to be a typevar
    bool isAlreadyASubType(const GlobalState &gs, const TypePtr &, const TypePtr &) const;
    // returns true if was successfully solved
    bool solve(const GlobalState &gs);
    TypePtr getInstantiation(SymbolRef) const;
    std::unique_ptr<TypeConstraint> deepCopy() const;
    InlinedVector<SymbolRef, 4> getDomain() const;
    static TypeConstraint EmptyFrozenConstraint;
    std::string toString(const core::GlobalState &gs) const;

    std::vector<ErrorLine> toExplanation(const core::GlobalState &gs) const;
};

} // namespace sorbet::core

#endif // SORBET_TYPECONSTRAINT_H
