#ifndef SORBET_TYPECONSTRAINT_H
#define SORBET_TYPECONSTRAINT_H

#include "Types.h"
#include <unordered_map>
namespace sorbet::core {

class TypeConstraint {
    static TypeConstraint makeEmptyFrozenConstraint();
    std::vector<std::pair<SymbolRef, std::shared_ptr<Type>>> upperBounds;
    std::vector<std::pair<SymbolRef, std::shared_ptr<Type>>> lowerBounds;
    std::vector<std::pair<SymbolRef, std::shared_ptr<Type>>> solution;
    bool wasSolved = false;
    bool cantSolve = false;
    std::shared_ptr<Type> &findUpperBound(SymbolRef forWhat);
    std::shared_ptr<Type> &findLowerBound(SymbolRef forWhat);
    std::shared_ptr<Type> &findSolution(SymbolRef forWhat);

public:
    TypeConstraint() = default;
    TypeConstraint(const TypeConstraint &) = delete;
    TypeConstraint(TypeConstraint &&) = default;
    void defineDomain(Context ctx, const InlinedVector<SymbolRef, 4> &typeParams);
    bool hasUpperBound(SymbolRef forWhat) const;
    bool hasLowerBound(SymbolRef forWhat) const;
    std::shared_ptr<Type> findSolution(SymbolRef forWhat) const;
    std::shared_ptr<Type> findUpperBound(SymbolRef forWhat) const;
    std::shared_ptr<Type> findLowerBound(SymbolRef forWhat) const;

    bool isEmpty() const;
    inline bool isSolved() const {
        return wasSolved;
    }

    // At least one of arguments has to be a typevar
    bool rememberIsSubtype(Context ctx, const std::shared_ptr<Type> &, const std::shared_ptr<Type> &);

    // At least one of arguments has to be a typevar
    bool isAlreadyASubType(Context ctx, const std::shared_ptr<Type> &, const std::shared_ptr<Type> &) const;
    // returns true if was successfully solved
    bool solve(Context ctx);
    std::shared_ptr<Type> getInstantiation(SymbolRef) const;
    std::shared_ptr<TypeConstraint> deepCopy() const;
    static TypeConstraint EmptyFrozenConstraint;
};

} // namespace sorbet::core

#endif // SORBET_TYPECONSTRAINT_H
