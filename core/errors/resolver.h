#ifndef SORBET_CORE_ERRORS_RESOLVER_H
#define SORBET_CORE_ERRORS_RESOLVER_H
#include "core/Error.h"

namespace sorbet::core::errors::Resolver {
constexpr ErrorClass DynamicConstant{5001, StrictLevel::Typed};
constexpr ErrorClass StubConstant{5002, StrictLevel::Stripe};
constexpr ErrorClass InvalidMethodSignature{5003, StrictLevel::Stripe};
constexpr ErrorClass InvalidTypeDeclaration{5004, StrictLevel::Stripe};
constexpr ErrorClass InvalidDeclareVariables{5005, StrictLevel::Typed};
constexpr ErrorClass DuplicateVariableDeclaration{5006, StrictLevel::Typed};
constexpr ErrorClass UndeclaredVariable{5007, StrictLevel::Strict};
constexpr ErrorClass DynamicSuperclass{5008, StrictLevel::Typed};
/* constexpr ErrorClass InvalidAttr{5009, StrictLevel::Typed}; */
constexpr ErrorClass InvalidCast{5010, StrictLevel::Stripe};
constexpr ErrorClass CircularDependency{5011, StrictLevel::Stripe};
constexpr ErrorClass RedefinitionOfParents{5012, StrictLevel::Stripe};
constexpr ErrorClass ConstantAssertType{5013, StrictLevel::Stripe};
constexpr ErrorClass ParentTypeNotDeclared{5014, StrictLevel::Typed};
constexpr ErrorClass ParentVarianceMismatch{5015, StrictLevel::Stripe};
constexpr ErrorClass VariantTypeMemberInClass{5016, StrictLevel::Stripe};
constexpr ErrorClass TypeMembersInWrongOrder{5017, StrictLevel::Stripe};
constexpr ErrorClass NotATypeVariable{5018, StrictLevel::Stripe};
constexpr ErrorClass AbstractMethodWithBody{5019, StrictLevel::Stripe};
constexpr ErrorClass InvalidMixinDeclaration{5020, StrictLevel::Stripe};
constexpr ErrorClass AbstractMethodOutsideAbstract{5021, StrictLevel::Stripe};
constexpr ErrorClass ConcreteMethodInInterface{5022, StrictLevel::Stripe};
constexpr ErrorClass BadAbstractMethod{5023, StrictLevel::Typed}; // there are violations.
constexpr ErrorClass RecursiveTypeAlias{5024, StrictLevel::Stripe};
constexpr ErrorClass TypeAliasInGenericClass{5025, StrictLevel::Stripe};
constexpr ErrorClass BadStdlibGeneric{5026, StrictLevel::Stripe};

// This is for type signatures that we permit at Stripe but ban in Typed code
constexpr ErrorClass InvalidTypeDeclarationTyped{5027, StrictLevel::Typed};
constexpr ErrorClass ConstantMissingTypeAnnotation{5028, StrictLevel::Strict};
constexpr ErrorClass RecursiveClassAlias{5030, StrictLevel::Stripe};
constexpr ErrorClass ConstantInTypeAlias{5031, StrictLevel::Stripe};
constexpr ErrorClass IncludesNonModule{5032, StrictLevel::Stripe};
constexpr ErrorClass OverridesFinal{5033, StrictLevel::Stripe};
constexpr ErrorClass ReassignsTypeAlias{5034, StrictLevel::Stripe};
} // namespace sorbet::core::errors::Resolver

#endif
