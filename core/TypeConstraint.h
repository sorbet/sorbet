#ifndef SORBET_TYPECONSTRAINT_H
#define SORBET_TYPECONSTRAINT_H

#include "absl/types/span.h"
#include "core/Context.h"
#include "core/SymbolRef.h"
#include "core/TypePtr.h"
namespace sorbet::core {

class TypeConstraint {
    static TypeConstraint makeEmptyFrozenConstraint();
    std::vector<std::pair<TypeArgumentRef, TypePtr>> upperBounds;
    std::vector<std::pair<TypeArgumentRef, TypePtr>> lowerBounds;
    std::vector<std::pair<TypeArgumentRef, TypePtr>> solution;
    bool wasSolved = false;
    bool cantSolve = false;
    TypePtr &findUpperBound(TypeArgumentRef forWhat);
    TypePtr &findLowerBound(TypeArgumentRef forWhat);
    TypePtr &findSolution(TypeArgumentRef forWhat);

    UnorderedMap<TypeArgumentRef, std::pair<TypePtr, TypePtr>> collateBounds(const GlobalState &gs) const;

public:
    TypeConstraint() = default;
    TypeConstraint(const TypeConstraint &) = delete;
    TypeConstraint(TypeConstraint &&) = default;
    void defineDomain(const GlobalState &gs, absl::Span<const TypeArgumentRef> typeParams);
    bool hasUpperBound(TypeArgumentRef forWhat) const;
    bool hasLowerBound(TypeArgumentRef forWhat) const;
    TypePtr findSolution(TypeArgumentRef forWhat) const;
    TypePtr findUpperBound(TypeArgumentRef forWhat) const;
    TypePtr findLowerBound(TypeArgumentRef forWhat) const;

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
    TypePtr getInstantiation(TypeArgumentRef) const;
    std::unique_ptr<TypeConstraint> deepCopy() const;
    InlinedVector<SymbolRef, 4> getDomain() const;
    static TypeConstraint EmptyFrozenConstraint;
    std::string toString(const core::GlobalState &gs) const;

    ErrorSection explain(const core::GlobalState &gs) const;
};

} // namespace sorbet::core

#endif // SORBET_TYPECONSTRAINT_H
