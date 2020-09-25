#ifndef SORBET_CORE_ERRORS_INFER_H
#define SORBET_CORE_ERRORS_INFER_H
#include "core/Error.h"

namespace sorbet::core::errors::Infer {
// N.B infer does not run for untyped call at all. StrictLevel::False here would be meaningless
constexpr ErrorClass PinnedVariableMismatch{7001, StrictLevel::True};
constexpr ErrorClass MethodArgumentMismatch{7002, StrictLevel::True};
constexpr ErrorClass UnknownMethod{7003, StrictLevel::True};
constexpr ErrorClass MethodArgumentCountMismatch{7004, StrictLevel::True};
constexpr ErrorClass ReturnTypeMismatch{7005, StrictLevel::True};
constexpr ErrorClass DeadBranchInferencer{7006, StrictLevel::True};
constexpr ErrorClass CastTypeMismatch{7007, StrictLevel::True};
constexpr ErrorClass OverloadedArgumentCountMismatch{7008, StrictLevel::True};
constexpr ErrorClass BareTypeUsage{7009, StrictLevel::True};
constexpr ErrorClass GenericArgumentCountMismatch{7010, StrictLevel::True};
constexpr ErrorClass IncompleteType{7011, StrictLevel::True};
constexpr ErrorClass GlobalReassignmentTypeMismatch{7012, StrictLevel::True};
constexpr ErrorClass FieldReassignmentTypeMismatch{7013, StrictLevel::True};
// constexpr ErrorClass GenericMethodConstaintUnsolved{7013, StrictLevel::True};
constexpr ErrorClass RevealType{7014, StrictLevel::True};
constexpr ErrorClass InvalidCast{7015, StrictLevel::Strict};
constexpr ErrorClass ExpectedLiteralType{7016, StrictLevel::True};
constexpr ErrorClass UntypedMethod{7017, StrictLevel::Strict};
constexpr ErrorClass UntypedValue{7018, StrictLevel::Strong};
constexpr ErrorClass UntypedSplat{7019, StrictLevel::True};
constexpr ErrorClass GenericMethodConstaintUnsolved{7020, StrictLevel::True};
constexpr ErrorClass BlockNotPassed{7021, StrictLevel::True};
constexpr ErrorClass SuggestTyped{7022, StrictLevel::True};
constexpr ErrorClass ProcArityUnknown{7023, StrictLevel::Strict};
constexpr ErrorClass GenericPassedAsBlock{7024, StrictLevel::True};
constexpr ErrorClass AbstractClassInstantiated{7025, StrictLevel::True};
constexpr ErrorClass NotExhaustive{7026, StrictLevel::True};
constexpr ErrorClass UntypedConstantSuggestion{7027, StrictLevel::Strict};
// constexpr ErrorClass GenericTypeParamBoundMismatch{7028, StrictLevel::False};
// constexpr ErrorClass LazyResolve{7029, StrictLevel::True};
constexpr ErrorClass MetaTypeDispatchCall{7030, StrictLevel::True};
// N.B infer does not run for untyped call at all. StrictLevel::False here would be meaningless
} // namespace sorbet::core::errors::Infer
#endif
