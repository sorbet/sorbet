#ifndef SORBET_CORE_ERRORS_INFER_H
#define SORBET_CORE_ERRORS_INFER_H
#include "core/Error.h"
#include "core/TypePtr.h"

namespace sorbet::core::errors::Infer {
// N.B infer does not run for untyped call at all. StrictLevel::False here would be meaningless
inline constexpr ErrorClass PinnedVariableMismatch{7001, StrictLevel::True};
inline constexpr ErrorClass MethodArgumentMismatch{7002, StrictLevel::True};
inline constexpr ErrorClass UnknownMethod{7003, StrictLevel::True};
inline constexpr ErrorClass MethodArgumentCountMismatch{7004, StrictLevel::True};
inline constexpr ErrorClass ReturnTypeMismatch{7005, StrictLevel::True};
inline constexpr ErrorClass DeadBranchInferencer{7006, StrictLevel::True};
inline constexpr ErrorClass CastTypeMismatch{7007, StrictLevel::True};
// inline constexpr ErrorClass OverloadedArgumentCountMismatch{7008, StrictLevel::True};
inline constexpr ErrorClass BareTypeUsage{7009, StrictLevel::True};
inline constexpr ErrorClass GenericArgumentCountMismatch{7010, StrictLevel::True};
inline constexpr ErrorClass IncompleteType{7011, StrictLevel::True};
// inline constexpr ErrorClass GlobalReassignmentTypeMismatch{7012, StrictLevel::True};
inline constexpr ErrorClass FieldReassignmentTypeMismatch{7013, StrictLevel::True};
// inline constexpr ErrorClass GenericMethodConstraintUnsolved{7013, StrictLevel::True};
inline constexpr ErrorClass RevealType{7014, StrictLevel::True};
inline constexpr ErrorClass InvalidCast{7015, StrictLevel::Strict};
inline constexpr ErrorClass ExpectedLiteralType{7016, StrictLevel::True};
inline constexpr ErrorClass UntypedMethod{7017, StrictLevel::Strict};
inline constexpr ErrorClass UntypedValue{7018, StrictLevel::Strong};
inline constexpr ErrorClass UntypedSplat{7019, StrictLevel::True};
inline constexpr ErrorClass GenericMethodConstraintUnsolved{7020, StrictLevel::True};
inline constexpr ErrorClass BlockNotPassed{7021, StrictLevel::True};
inline constexpr ErrorClass SuggestTyped{7022, StrictLevel::True};
inline constexpr ErrorClass ProcArityUnknown{7023, StrictLevel::Strict};
inline constexpr ErrorClass GenericPassedAsBlock{7024, StrictLevel::True};
// inline constexpr ErrorClass AbstractClassInstantiated{7025, StrictLevel::True};
inline constexpr ErrorClass NotExhaustive{7026, StrictLevel::True};
inline constexpr ErrorClass UntypedConstantSuggestion{7027, StrictLevel::Strict};
// inline constexpr ErrorClass GenericTypeParamBoundMismatch{7028, StrictLevel::False};
// inline constexpr ErrorClass LazyResolve{7029, StrictLevel::True};
inline constexpr ErrorClass MetaTypeDispatchCall{7030, StrictLevel::True};
inline constexpr ErrorClass PrivateMethod{7031, StrictLevel::True};
inline constexpr ErrorClass GenericArgumentKeywordArgs{7032, StrictLevel::True};
inline constexpr ErrorClass KeywordArgHashWithoutSplat{7033, StrictLevel::True};
inline constexpr ErrorClass UnnecessarySafeNavigation{7034, StrictLevel::True};
inline constexpr ErrorClass TakesNoBlock{7035, StrictLevel::True};
inline constexpr ErrorClass PackagePrivateMethod{7036, StrictLevel::True};
inline constexpr ErrorClass CallAfterAndAnd{7037, StrictLevel::True};
inline constexpr ErrorClass CallOnTypeArgument{7038, StrictLevel::True};
inline constexpr ErrorClass CallOnUnboundedTypeMember{7039, StrictLevel::True};
inline constexpr ErrorClass AttachedClassOnInstance{7040, StrictLevel::True};
inline constexpr ErrorClass UntypedFieldSuggestion{7043, StrictLevel::Strict};
inline constexpr ErrorClass DigExtraArgs{7044, StrictLevel::True};
inline constexpr ErrorClass IncorrectlyAssumedType{7045, StrictLevel::True};
inline constexpr ErrorClass NonOverlappingEqual{7046, StrictLevel::True};
inline constexpr ErrorClass UntypedValueInformation{7047, StrictLevel::True};
inline constexpr ErrorClass UnknownSuperMethod{7048, StrictLevel::True};
inline constexpr ErrorClass TypecheckOverloadBody{7049, StrictLevel::True};
inline constexpr ErrorClass RedundantMust{7050, StrictLevel::Strict};
inline constexpr ErrorClass BranchOnVoid{7051, StrictLevel::True};
// N.B infer does not run for untyped call at all. StrictLevel::False here would be meaningless

ErrorClass errorClassForUntyped(const GlobalState &gs, FileRef file, const TypePtr &ptr);

} // namespace sorbet::core::errors::Infer
#endif
