#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Resolver {
constexpr ErrorClass DynamicConstant = 5001;
constexpr ErrorClass StubConstant = 5002;
constexpr ErrorClass InvalidMethodSignature = 5003;
constexpr ErrorClass InvalidTypeDeclaration = 5004;
constexpr ErrorClass InvalidDeclareVariables = 5005;
constexpr ErrorClass DuplicateVariableDeclaration = 5006;
constexpr ErrorClass UndeclaredVariable = 5007;
constexpr ErrorClass DynamicSuperclass = 5008;
constexpr ErrorClass InvalidAttr = 5009;
constexpr ErrorClass InvalidCast = 5010;
constexpr ErrorClass CircularDependency = 5011;
constexpr ErrorClass RedefinitionOfParents = 5012;
constexpr ErrorClass ConstantAssertType = 5013;
} // namespace Resolver
} // namespace errors
} // namespace core
} // namespace ruby_typer
