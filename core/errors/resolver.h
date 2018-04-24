#ifndef SRUBY_CORE_ERRORS_RESOLVER_H
#define SRUBY_CORE_ERRORS_RESOLVER_H
#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Resolver {
constexpr ErrorClass DynamicConstant{5001, StrictLevel::Typed};
constexpr ErrorClass StubConstant{5002, StrictLevel::Typed};
constexpr ErrorClass InvalidMethodSignature{5003, StrictLevel::Stripe};
constexpr ErrorClass InvalidTypeDeclaration{5004, StrictLevel::Stripe};
constexpr ErrorClass InvalidDeclareVariables{5005, StrictLevel::Typed};
constexpr ErrorClass DuplicateVariableDeclaration{5006, StrictLevel::Typed};
constexpr ErrorClass UndeclaredVariable{5007, StrictLevel::Strict};
constexpr ErrorClass DynamicSuperclass{5008, StrictLevel::Typed};
/* constexpr ErrorClass InvalidAttr{5009, StrictLevel::Typed}; */
constexpr ErrorClass InvalidCast{5010, StrictLevel::Typed};
constexpr ErrorClass CircularDependency{5011, StrictLevel::Stripe};
constexpr ErrorClass RedefinitionOfParents{5012, StrictLevel::Stripe};
constexpr ErrorClass ConstantAssertType{5013, StrictLevel::Stripe};
constexpr ErrorClass ParentTypeNotDeclared{5014, StrictLevel::Typed};
constexpr ErrorClass ParentVarianceMismatch{5015, StrictLevel::Typed};
constexpr ErrorClass VariantTypeMemberInClass{5016, StrictLevel::Typed};
constexpr ErrorClass TypeMembersInWrongOrder{5017, StrictLevel::Typed};
constexpr ErrorClass NotATypeVariable{5018, StrictLevel::Typed};
constexpr ErrorClass AbstractMethodWithBody{5019, StrictLevel::Typed};
constexpr ErrorClass InvalidMixinDeclaration{5020, StrictLevel::Typed};
constexpr ErrorClass AbstractMethodOutsideAbstract{5021, StrictLevel::Stripe};
constexpr ErrorClass ConcreteMethodInInterface{5022, StrictLevel::Stripe};
constexpr ErrorClass BadAbstractMethod{5023, StrictLevel::Typed};
} // namespace Resolver
} // namespace errors
} // namespace core
} // namespace ruby_typer

#endif
