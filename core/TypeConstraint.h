#ifndef SORBET_TYPECONSTRAINT_H
#define SORBET_TYPECONSTRAINT_H

#include "Types.h"
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

public:
    TypeConstraint() = default;
    TypeConstraint(const TypeConstraint &) = delete;
    TypeConstraint(TypeConstraint &&) = default;
    void defineDomain(Context ctx, const InlinedVector<SymbolRef, 4> &typeParams);
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
    bool rememberIsSubtype(Context ctx, const TypePtr &, const TypePtr &);

    // At least one of arguments has to be a typevar
    bool isAlreadyASubType(Context ctx, const TypePtr &, const TypePtr &) const;
    // returns true if was successfully solved
    bool solve(Context ctx);
    TypePtr getInstantiation(SymbolRef) const;
    std::shared_ptr<TypeConstraint> deepCopy() const;
    InlinedVector<SymbolRef, 4> getDomain() const;
    static TypeConstraint EmptyFrozenConstraint;
    std::string toString(Context ctx) const;
};

} // namespace sorbet::core

#endif // SORBET_TYPECONSTRAINT_H
