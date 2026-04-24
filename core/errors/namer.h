#ifndef SORBET_CORE_ERRORS_NAMER_H
#define SORBET_CORE_ERRORS_NAMER_H
#include "core/Error.h"

namespace sorbet::core::errors::Namer {
inline constexpr ErrorClass IncludeMultipleParam{4001, StrictLevel::False};
inline constexpr ErrorClass AncestorNotConstant{4002, StrictLevel::False};
inline constexpr ErrorClass IncludePassedBlock{4003, StrictLevel::False};
// inline constexpr ErrorClass DynamicConstantDefinition{4004, StrictLevel::True};
// inline constexpr ErrorClass DynamicMethodDefinition{4005, StrictLevel::True};
inline constexpr ErrorClass SuperOutsideMethod{4006, StrictLevel::True};
// inline constexpr ErrorClass DynamicDSLInvocation{4007, StrictLevel::True};
// inline constexpr ErrorClass MethodNotFound{4008, StrictLevel::True};
// inline constexpr ErrorClass InvalidAlias{4009, StrictLevel::True};
inline constexpr ErrorClass RedefinitionOfMethod{4010, StrictLevel::True};
inline constexpr ErrorClass InvalidTypeDefinition{4011, StrictLevel::False};
inline constexpr ErrorClass ModuleKindRedefinition{4012, StrictLevel::False};
inline constexpr ErrorClass InterfaceClass{4013, StrictLevel::False};
inline constexpr ErrorClass DynamicConstant{4014, StrictLevel::False};
// inline constexpr ErrorClass InvalidClassOwner{4015, StrictLevel::False};
inline constexpr ErrorClass RootTypeMember{4016, StrictLevel::False};
// inline constexpr ErrorClass DynamicConstantAssignment{4017, StrictLevel::False};
// inline constexpr ErrorClass RepeatedArgument{4018, StrictLevel::False};
inline constexpr ErrorClass MultipleBehaviorDefs{4019, StrictLevel::False};
// inline constexpr ErrorClass YAMLSyntaxError{4020, StrictLevel::False};
inline constexpr ErrorClass OldTypeMemberSyntax{4021, StrictLevel::False};
inline constexpr ErrorClass ConstantKindRedefinition{4022, StrictLevel::False};
// inline constexpr ErrorClass HasAttachedClassInClass{4023, StrictLevel::False};
inline constexpr ErrorClass DuplicateKeywordArg{4024, StrictLevel::False};
inline constexpr ErrorClass PackagePrivateOutsidePackage{4025, StrictLevel::False};
inline constexpr ErrorClass PackageScopeMustBeClass{4026, StrictLevel::False};
inline constexpr ErrorClass RedefinitionOfPackage{4027, StrictLevel::False};
inline constexpr ErrorClass ModifyingUnpackagedConstant{4028, StrictLevel::False};
inline constexpr ErrorClass InvalidPackageExpression{4029, StrictLevel::False};
} // namespace sorbet::core::errors::Namer

#endif
