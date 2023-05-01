#ifndef SORBET_CORE_ERRORS_NAMER_H
#define SORBET_CORE_ERRORS_NAMER_H
#include "core/Error.h"

namespace sorbet::core::errors::Namer {
constexpr ErrorClass IncludeMutipleParam{4001, StrictLevel::False};
constexpr ErrorClass AncestorNotConstant{4002, StrictLevel::False};
constexpr ErrorClass IncludePassedBlock{4003, StrictLevel::False};
// constexpr ErrorClass DynamicConstantDefinition{4004, StrictLevel::True};
// constexpr ErrorClass DynamicMethodDefinition{4005, StrictLevel::True};
constexpr ErrorClass SuperOutsideMethod{4006, StrictLevel::True};
// constexpr ErrorClass DynamicDSLInvocation{4007, StrictLevel::True};
// constexpr ErrorClass MethodNotFound{4008, StrictLevel::True};
// constexpr ErrorClass InvalidAlias{4009, StrictLevel::True};
constexpr ErrorClass RedefinitionOfMethod{4010, StrictLevel::True};
constexpr ErrorClass InvalidTypeDefinition{4011, StrictLevel::True};
constexpr ErrorClass ModuleKindRedefinition{4012, StrictLevel::False};
constexpr ErrorClass InterfaceClass{4013, StrictLevel::False};
constexpr ErrorClass DynamicConstant{4014, StrictLevel::False};
constexpr ErrorClass InvalidClassOwner{4015, StrictLevel::False};
constexpr ErrorClass RootTypeMember{4016, StrictLevel::False};
// constexpr ErrorClass DynamicConstantAssignment{4017, StrictLevel::False};
// constexpr ErrorClass RepeatedArgument{4018, StrictLevel::False};
constexpr ErrorClass MultipleBehaviorDefs{4019, StrictLevel::False};
// constexpr ErrorClass YAMLSyntaxError{4020, StrictLevel::False};
constexpr ErrorClass OldTypeMemberSyntax{4021, StrictLevel::False};
constexpr ErrorClass ConstantKindRedefinition{4022, StrictLevel::False};
constexpr ErrorClass HasAttachedClassInClass{4023, StrictLevel::False};
} // namespace sorbet::core::errors::Namer

#endif
