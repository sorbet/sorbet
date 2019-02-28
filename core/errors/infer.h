#ifndef SORBET_CORE_ERRORS_INFER_H
#define SORBET_CORE_ERRORS_INFER_H
#include "core/Error.h"

namespace sorbet::core::errors::Infer {
// N.B infer does not run for untyped call at all. Level::Ruby here would be meaningless
constexpr ErrorClass PinnedVariableMismatch{7001, StrictLevel::Typed};
constexpr ErrorClass MethodArgumentMismatch{7002, StrictLevel::Typed};
constexpr ErrorClass UnknownMethod{7003, StrictLevel::Typed};
constexpr ErrorClass MethodArgumentCountMismatch{7004, StrictLevel::Typed};
constexpr ErrorClass ReturnTypeMismatch{7005, StrictLevel::Typed};
constexpr ErrorClass DeadBranchInferencer{7006, StrictLevel::Typed};
constexpr ErrorClass CastTypeMismatch{7007, StrictLevel::Typed};
constexpr ErrorClass OverloadedArgumentCountMismatch{7008, StrictLevel::Typed};
constexpr ErrorClass BareTypeUsage{7009, StrictLevel::Typed};
constexpr ErrorClass GenericArgumentCountMismatch{7010, StrictLevel::Typed};
constexpr ErrorClass IncompleteType{7011, StrictLevel::Typed};
constexpr ErrorClass GlobalReassignmentTypeMismatch{7012, StrictLevel::Typed};
constexpr ErrorClass FieldReassignmentTypeMismatch{7013, StrictLevel::Typed};
// constexpr ErrorClass GenericMethodConstaintUnsolved{7013, StrictLevel::Typed};
constexpr ErrorClass RevealType{7014, StrictLevel::Typed};
constexpr ErrorClass InvalidCast{7015, StrictLevel::Strict};
constexpr ErrorClass ExpectedLiteralType{7016, StrictLevel::Typed};
constexpr ErrorClass UntypedMethod{7017, StrictLevel::Strict};
constexpr ErrorClass UntypedValue{7018, StrictLevel::Strong};
constexpr ErrorClass UntypedSplat{7019, StrictLevel::Typed};
constexpr ErrorClass GenericMethodConstaintUnsolved{7020, StrictLevel::Typed};
constexpr ErrorClass BlockNotPassed{7021, StrictLevel::Typed};
constexpr ErrorClass SuggestTyped{7022, StrictLevel::Typed};
} // namespace sorbet::core::errors::Infer
#endif
