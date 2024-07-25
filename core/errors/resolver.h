#ifndef SORBET_CORE_ERRORS_RESOLVER_H
#define SORBET_CORE_ERRORS_RESOLVER_H
#include "core/Error.h"

namespace sorbet::core::errors::Resolver {
constexpr ErrorClass DynamicConstant{5001, StrictLevel::True};
constexpr ErrorClass StubConstant{5002, StrictLevel::False};
constexpr ErrorClass InvalidMethodSignature{5003, StrictLevel::False};
constexpr ErrorClass InvalidTypeDeclaration{5004, StrictLevel::False};
constexpr ErrorClass InvalidDeclareVariables{5005, StrictLevel::False};
constexpr ErrorClass DuplicateVariableDeclaration{5006, StrictLevel::True};
// constexpr ErrorClass UndeclaredVariable{5007, StrictLevel::Strict};
constexpr ErrorClass DynamicSuperclass{5008, StrictLevel::True};
// constexpr ErrorClass InvalidAttr{5009, StrictLevel::True};
// constexpr ErrorClass InvalidCast{5010, StrictLevel::False};
constexpr ErrorClass CircularDependency{5011, StrictLevel::False};
constexpr ErrorClass RedefinitionOfParents{5012, StrictLevel::False};
constexpr ErrorClass ConstantAssertType{5013, StrictLevel::False};
constexpr ErrorClass ParentTypeNotDeclared{5014, StrictLevel::False};
constexpr ErrorClass ParentVarianceMismatch{5015, StrictLevel::False};
// constexpr ErrorClass VariantTypeMemberInClass{5016, StrictLevel::False};
constexpr ErrorClass TypeMembersInWrongOrder{5017, StrictLevel::False};
constexpr ErrorClass NotATypeVariable{5018, StrictLevel::False};
constexpr ErrorClass AbstractMethodWithBody{5019, StrictLevel::False};
constexpr ErrorClass InvalidMixinDeclaration{5020, StrictLevel::False};
constexpr ErrorClass AbstractMethodOutsideAbstract{5021, StrictLevel::False};
constexpr ErrorClass ConcreteMethodInInterface{5022, StrictLevel::False};
constexpr ErrorClass BadAbstractMethod{5023, StrictLevel::False};
constexpr ErrorClass RecursiveTypeAlias{5024, StrictLevel::False};
// constexpr ErrorClass TypeAliasInGenericClass{5025, StrictLevel::False};
constexpr ErrorClass BadStdlibGeneric{5026, StrictLevel::False};
constexpr ErrorClass OutOfOrderConstantAccess{5027, StrictLevel::False};

// constexpr ErrorClass InvalidTypeDeclarationTyped{5027, StrictLevel::True};
// constexpr ErrorClass ConstantMissingTypeAnnotation{5028, StrictLevel::Strict};
constexpr ErrorClass RecursiveClassAlias{5030, StrictLevel::False};
constexpr ErrorClass ConstantInTypeAlias{5031, StrictLevel::False};
constexpr ErrorClass IncludesNonModule{5032, StrictLevel::False};
constexpr ErrorClass OverridesFinal{5033, StrictLevel::False};
constexpr ErrorClass ReassignsTypeAlias{5034, StrictLevel::False};
constexpr ErrorClass BadMethodOverride{5035, StrictLevel::False};
constexpr ErrorClass EnumerableParentTypeNotDeclared{5036, StrictLevel::Strict};
constexpr ErrorClass BadAliasMethod{5037, StrictLevel::True};
constexpr ErrorClass SigInFileWithoutSigil{5038, StrictLevel::False};
constexpr ErrorClass RevealTypeInUntypedFile{5039, StrictLevel::False};

constexpr ErrorClass OverloadNotAllowed{5040, StrictLevel::False};
constexpr ErrorClass SubclassingNotAllowed{5041, StrictLevel::False};
constexpr ErrorClass NonPublicAbstract{5042, StrictLevel::True};
constexpr ErrorClass InvalidTypeAlias{5043, StrictLevel::False};
constexpr ErrorClass InvalidVariance{5044, StrictLevel::True};
constexpr ErrorClass GenericClassWithoutTypeArgs{5045, StrictLevel::False};
constexpr ErrorClass GenericClassWithoutTypeArgsStdlib{5046, StrictLevel::Strict};
constexpr ErrorClass FinalAncestor{5047, StrictLevel::False};
constexpr ErrorClass FinalModuleNonFinalMethod{5048, StrictLevel::False};
constexpr ErrorClass BadParameterOrdering{5049, StrictLevel::False};
constexpr ErrorClass SealedAncestor{5050, StrictLevel::False};
constexpr ErrorClass UndeclaredOverride{5051, StrictLevel::True};
constexpr ErrorClass InvalidTypeMemberBounds{5052, StrictLevel::False};
constexpr ErrorClass ParentTypeBoundsMismatch{5053, StrictLevel::False};
constexpr ErrorClass ImplementationDeprecated{5054, StrictLevel::False};
constexpr ErrorClass TypeMemberCycle{5055, StrictLevel::False};
constexpr ErrorClass ExperimentalAttachedClass{5056, StrictLevel::False};
// constexpr ErrorClass GeneratedDeprecated{5056, StrictLevel::False};
constexpr ErrorClass StaticAbstractModuleMethod{5057, StrictLevel::False};
constexpr ErrorClass AttachedClassAsParam{5058, StrictLevel::False};
constexpr ErrorClass LazyResolve{5059, StrictLevel::False};
constexpr ErrorClass GenericTypeParamBoundMismatch{5060, StrictLevel::False};
constexpr ErrorClass PrivateConstantReferenced{5061, StrictLevel::True};
constexpr ErrorClass InvalidRequiredAncestor{5062, StrictLevel::True};
constexpr ErrorClass UselessRequiredAncestor{5063, StrictLevel::True};
constexpr ErrorClass UnsatisfiedRequiredAncestor{5064, StrictLevel::True};
constexpr ErrorClass UnsatisfiableRequiredAncestor{5065, StrictLevel::True};
// constexpr ErrorClass ExperimentalRequiredAncestor{5066, StrictLevel::False};
constexpr ErrorClass NonClassSuperclass{5067, StrictLevel::False};
constexpr ErrorClass AmbiguousDefinitionError{5068, StrictLevel::False};
constexpr ErrorClass MultipleStatementsInSig{5069, StrictLevel::False};
constexpr ErrorClass NilableUntyped{5070, StrictLevel::False};
constexpr ErrorClass BindNonBlockParameter{5071, StrictLevel::False};
constexpr ErrorClass TypeMemberScopeMismatch{5072, StrictLevel::False};
constexpr ErrorClass AbstractClassInstantiated{5073, StrictLevel::True};
constexpr ErrorClass HasAttachedClassIncluded{5074, StrictLevel::False};
constexpr ErrorClass TypeAliasToTypeMember{5075, StrictLevel::False};
constexpr ErrorClass GenericArgumentCountMismatch{5076, StrictLevel::True};
constexpr ErrorClass GenericArgumentKeywordArgs{5077, StrictLevel::True};
} // namespace sorbet::core::errors::Resolver

#endif
