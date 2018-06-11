#include "core/Errors.h"

namespace sorbet {
namespace core {
namespace errors {
namespace Infer {
// N.B infer does not run for untyped call at all. Level::Ruby here would be meaningless
constexpr ErrorClass PinnedVariableMismatch{7001, core::StrictLevel::Typed};
constexpr ErrorClass MethodArgumentMismatch{7002, core::StrictLevel::Typed};
constexpr ErrorClass UnknownMethod{7003, core::StrictLevel::Typed};
constexpr ErrorClass MethodArgumentCountMismatch{7004, core::StrictLevel::Typed};
constexpr ErrorClass ReturnTypeMismatch{7005, core::StrictLevel::Typed};
constexpr ErrorClass DeadBranchInferencer{7006, core::StrictLevel::Typed};
constexpr ErrorClass CastTypeMismatch{7007, core::StrictLevel::Typed};
constexpr ErrorClass OverloadedArgumentCountMismatch{7008, core::StrictLevel::Typed};
constexpr ErrorClass BareTypeUsage{7009, core::StrictLevel::Typed};
constexpr ErrorClass GenericArgumentCountMismatch{7010, core::StrictLevel::Typed};
constexpr ErrorClass IncompleteType{7011, core::StrictLevel::Typed};
constexpr ErrorClass GlobalReassignmentTypeMismatch{7012, core::StrictLevel::Typed};
constexpr ErrorClass FieldReassignmentTypeMismatch{7013, core::StrictLevel::Typed};
constexpr ErrorClass GenericMethodConstaintUnsolved{7013, core::StrictLevel::Typed};

} // namespace Infer
} // namespace errors
} // namespace core
} // namespace sorbet
