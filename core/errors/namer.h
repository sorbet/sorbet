#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Namer {
constexpr ErrorClass IncludeMutipleParam = 4001;
constexpr ErrorClass IncludeNotConstant = 4002;
constexpr ErrorClass IncludePassedBlock = 4003;
constexpr ErrorClass DynamicConstantDefinition = 4004;
constexpr ErrorClass DynamicMethodDefinition = 4005;
constexpr ErrorClass SelfOutsideClass = 4006;
constexpr ErrorClass DynamicDSLInvocation = 4007;
constexpr ErrorClass MethodNotFound = 4008;
constexpr ErrorClass InvalidAlias = 4009;
constexpr ErrorClass RedefinitionOfMethod = 4010;
} // namespace Namer
} // namespace errors
} // namespace core
} // namespace ruby_typer
