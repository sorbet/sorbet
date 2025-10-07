#ifndef SORBET_TYPECONSTRAINT_H
#define SORBET_TYPECONSTRAINT_H

#include "absl/types/span.h"
#include "core/Context.h"
#include "core/SymbolRef.h"
#include "core/TypePtr.h"
namespace sorbet::core {

class TypeConstraint {
    static TypeConstraint makeEmptyFrozenConstraint();
    std::vector<std::pair<TypeParameterRef, TypePtr>> upperBounds;
    std::vector<std::pair<TypeParameterRef, TypePtr>> lowerBounds;
    std::vector<std::pair<TypeParameterRef, TypePtr>> solution;
    bool wasSolved = false;
    bool cantSolve = false;
    TypePtr &findUpperBound(TypeParameterRef forWhat);
    TypePtr &findLowerBound(TypeParameterRef forWhat);
    TypePtr &findSolution(TypeParameterRef forWhat);

    UnorderedMap<TypeParameterRef, std::pair<TypePtr, TypePtr>> collateBounds(const GlobalState &gs) const;

public:
    TypeConstraint() = default;
    TypeConstraint(const TypeConstraint &) = delete;
    TypeConstraint(TypeConstraint &&) = default;
    void defineDomain(const GlobalState &gs, absl::Span<const TypeParameterRef> typeParams);
    bool hasUpperBound(TypeParameterRef forWhat) const;
    bool hasLowerBound(TypeParameterRef forWhat) const;
    TypePtr findSolution(TypeParameterRef forWhat) const;
    TypePtr findUpperBound(TypeParameterRef forWhat) const;
    TypePtr findLowerBound(TypeParameterRef forWhat) const;

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
    TypePtr getInstantiation(TypeParameterRef) const;
    std::unique_ptr<TypeConstraint> deepCopy() const;
    InlinedVector<SymbolRef, 4> getDomain() const;
    static TypeConstraint EmptyFrozenConstraint;
    std::string toString(const core::GlobalState &gs) const;

    ErrorSection explain(const core::GlobalState &gs) const;
};

} // namespace sorbet::core

#endif // SORBET_TYPECONSTRAINT_H
