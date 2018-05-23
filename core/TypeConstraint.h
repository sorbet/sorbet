#ifndef SORBET_TYPECONSTRAINT_H
#define SORBET_TYPECONSTRAINT_H

#include "Types.h"
#include <unordered_map>
namespace ruby_typer {
namespace core {

class TypeConstraint {
    static TypeConstraint makeEmptyFrozenConstraint();
    std::vector<std::pair<core::SymbolRef, std::shared_ptr<Type>>> upperBounds;
    std::vector<std::pair<core::SymbolRef, std::shared_ptr<Type>>> lowerBounds;
    std::vector<std::pair<core::SymbolRef, std::shared_ptr<Type>>> solution;
    bool wasSolved = false;
    bool cantSolve = false;
    std::shared_ptr<Type> &findUpperBound(core::SymbolRef forWhat);
    std::shared_ptr<Type> &findLowerBound(core::SymbolRef forWhat);
    std::shared_ptr<Type> &findSolution(core::SymbolRef forWhat);

public:
    TypeConstraint() = default;
    TypeConstraint(const TypeConstraint &) = delete;
    TypeConstraint(TypeConstraint &&) = default;
    void defineDomain(Context ctx, const std::vector<SymbolRef> &typeParams);
    bool hasUpperBound(core::SymbolRef forWhat) const;
    bool hasLowerBound(core::SymbolRef forWhat) const;
    std::shared_ptr<Type> findSolution(core::SymbolRef forWhat) const;
    std::shared_ptr<Type> findUpperBound(core::SymbolRef forWhat) const;
    std::shared_ptr<Type> findLowerBound(core::SymbolRef forWhat) const;

    bool isEmpty() const;
    inline bool isSolved() const {
        return wasSolved;
    }

    // At least one of arguments has to be a typevar
    bool rememberIsSubtype(Context ctx, std::shared_ptr<Type>, std::shared_ptr<Type>);

    // At least one of arguments has to be a typevar
    bool isAlreadyASubType(Context ctx, std::shared_ptr<Type>, std::shared_ptr<Type>) const;
    // returns true if was successfully solved
    bool solve(Context ctx);
    std::shared_ptr<Type> getInstantiation(core::SymbolRef) const;
    std::shared_ptr<TypeConstraint> deepCopy() const;
    static TypeConstraint EmptyFrozenConstraint;
};

} // namespace core
} // namespace ruby_typer

#endif // SORBET_TYPECONSTRAINT_H
