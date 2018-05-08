#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Namer {
constexpr ErrorClass IncludeMutipleParam{4001, StrictLevel::Stripe};
constexpr ErrorClass IncludeNotConstant{4002, StrictLevel::Stripe};
constexpr ErrorClass IncludePassedBlock{4003, StrictLevel::Stripe};
constexpr ErrorClass DynamicConstantDefinition{4004, StrictLevel::Typed};
constexpr ErrorClass DynamicMethodDefinition{4005, StrictLevel::Typed};
constexpr ErrorClass SelfOutsideClass{4006, StrictLevel::Typed};
constexpr ErrorClass DynamicDSLInvocation{4007, StrictLevel::Typed};
constexpr ErrorClass MethodNotFound{4008, StrictLevel::Typed};
constexpr ErrorClass InvalidAlias{4009, StrictLevel::Typed};
constexpr ErrorClass RedefinitionOfMethod{4010, StrictLevel::Typed};
constexpr ErrorClass InvalidTypeDefinition{4011, StrictLevel::Typed};
constexpr ErrorClass ModuleKindRedefinition{4012, StrictLevel::Stripe};
constexpr ErrorClass InterfaceClass{4013, StrictLevel::Stripe};
} // namespace Namer
} // namespace errors
} // namespace core
} // namespace ruby_typer
