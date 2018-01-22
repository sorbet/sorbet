#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Infer {
constexpr ErrorClass PinnedVariableMismatch = 7001;
constexpr ErrorClass MethodArgumentMismatch = 7002;
constexpr ErrorClass UnknownMethod = 7003;
constexpr ErrorClass MethodArgumentCountMismatch = 7004;
constexpr ErrorClass ReturnTypeMismatch = 7005;
constexpr ErrorClass DeadBranchInferencer = 7006;
constexpr ErrorClass CastTypeMismatch = 7007;
constexpr ErrorClass OverloadedArgumentCountMismatch = 7008;

} // namespace Infer
} // namespace errors
} // namespace core
} // namespace ruby_typer
