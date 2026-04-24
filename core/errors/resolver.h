#ifndef SORBET_CORE_ERRORS_RESOLVER_H
#define SORBET_CORE_ERRORS_RESOLVER_H
#include "core/Error.h"

namespace sorbet::core::errors::Resolver {
inline constexpr ErrorClass DynamicConstant{5001, StrictLevel::True};
inline constexpr ErrorClass StubConstant{5002, StrictLevel::False};
inline constexpr ErrorClass InvalidMethodSignature{5003, StrictLevel::False};
inline constexpr ErrorClass InvalidTypeDeclaration{5004, StrictLevel::False};
inline constexpr ErrorClass InvalidDeclareVariables{5005, StrictLevel::False};
inline constexpr ErrorClass DuplicateVariableDeclaration{5006, StrictLevel::True};
// inline constexpr ErrorClass UndeclaredVariable{5007, StrictLevel::Strict};
inline constexpr ErrorClass DynamicSuperclass{5008, StrictLevel::True};
// inline constexpr ErrorClass InvalidAttr{5009, StrictLevel::True};
// inline constexpr ErrorClass InvalidCast{5010, StrictLevel::False};
inline constexpr ErrorClass CircularDependency{5011, StrictLevel::False};
inline constexpr ErrorClass RedefinitionOfParents{5012, StrictLevel::False};
inline constexpr ErrorClass ConstantAssertType{5013, StrictLevel::False};
inline constexpr ErrorClass ParentTypeNotDeclared{5014, StrictLevel::False};
inline constexpr ErrorClass ParentVarianceMismatch{5015, StrictLevel::False};
// inline constexpr ErrorClass VariantTypeMemberInClass{5016, StrictLevel::False};
inline constexpr ErrorClass TypeMembersInWrongOrder{5017, StrictLevel::False};
inline constexpr ErrorClass NotATypeVariable{5018, StrictLevel::False};
inline constexpr ErrorClass AbstractMethodWithBody{5019, StrictLevel::False};
inline constexpr ErrorClass InvalidMixinDeclaration{5020, StrictLevel::False};
inline constexpr ErrorClass AbstractMethodOutsideAbstract{5021, StrictLevel::False};
inline constexpr ErrorClass ConcreteMethodInInterface{5022, StrictLevel::False};
inline constexpr ErrorClass BadAbstractMethod{5023, StrictLevel::False};
inline constexpr ErrorClass RecursiveTypeAlias{5024, StrictLevel::False};
// inline constexpr ErrorClass TypeAliasInGenericClass{5025, StrictLevel::False};
inline constexpr ErrorClass BadStdlibGeneric{5026, StrictLevel::False};
inline constexpr ErrorClass OutOfOrderConstantAccess{5027, StrictLevel::False};

// inline constexpr ErrorClass InvalidTypeDeclarationTyped{5027, StrictLevel::True};
// inline constexpr ErrorClass ConstantMissingTypeAnnotation{5028, StrictLevel::Strict};
inline constexpr ErrorClass RecursiveClassAlias{5030, StrictLevel::False};
inline constexpr ErrorClass ConstantInTypeAlias{5031, StrictLevel::False};
inline constexpr ErrorClass IncludesNonModule{5032, StrictLevel::False};
inline constexpr ErrorClass OverridesFinal{5033, StrictLevel::False};
inline constexpr ErrorClass ReassignsTypeAlias{5034, StrictLevel::False};
inline constexpr ErrorClass BadMethodOverride{5035, StrictLevel::False};
inline constexpr ErrorClass EnumerableParentTypeNotDeclared{5036, StrictLevel::Strict};
inline constexpr ErrorClass BadAliasMethod{5037, StrictLevel::True};
inline constexpr ErrorClass SigInFileWithoutSigil{5038, StrictLevel::False};
inline constexpr ErrorClass RevealTypeInUntypedFile{5039, StrictLevel::False};

inline constexpr ErrorClass OverloadNotAllowed{5040, StrictLevel::False};
inline constexpr ErrorClass SubclassingNotAllowed{5041, StrictLevel::False};
inline constexpr ErrorClass NonPublicAbstract{5042, StrictLevel::True};
inline constexpr ErrorClass InvalidTypeAlias{5043, StrictLevel::False};
inline constexpr ErrorClass InvalidVariance{5044, StrictLevel::True};
inline constexpr ErrorClass GenericClassWithoutTypeArgs{5045, StrictLevel::False};
inline constexpr ErrorClass GenericClassWithoutTypeArgsStdlib{5046, StrictLevel::Strict};
inline constexpr ErrorClass FinalAncestor{5047, StrictLevel::False};
inline constexpr ErrorClass FinalModuleNonFinalMethod{5048, StrictLevel::False};
inline constexpr ErrorClass BadParameterOrdering{5049, StrictLevel::False};
inline constexpr ErrorClass SealedAncestor{5050, StrictLevel::False};
inline constexpr ErrorClass UndeclaredOverride{5051, StrictLevel::True};
inline constexpr ErrorClass InvalidTypeMemberBounds{5052, StrictLevel::False};
inline constexpr ErrorClass ParentTypeBoundsMismatch{5053, StrictLevel::False};
inline constexpr ErrorClass ImplementationDeprecated{5054, StrictLevel::False};
inline constexpr ErrorClass TypeMemberCycle{5055, StrictLevel::False};
inline constexpr ErrorClass ExperimentalAttachedClass{5056, StrictLevel::False};
// inline constexpr ErrorClass GeneratedDeprecated{5056, StrictLevel::False};
inline constexpr ErrorClass StaticAbstractModuleMethod{5057, StrictLevel::False};
inline constexpr ErrorClass AttachedClassAsParam{5058, StrictLevel::False};
// inline constexpr ErrorClass LazyResolve{5059, StrictLevel::False};
inline constexpr ErrorClass GenericTypeParamBoundMismatch{5060, StrictLevel::False};
inline constexpr ErrorClass PrivateConstantReferenced{5061, StrictLevel::True};
inline constexpr ErrorClass InvalidRequiredAncestor{5062, StrictLevel::True};
inline constexpr ErrorClass UselessRequiredAncestor{5063, StrictLevel::True};
inline constexpr ErrorClass UnsatisfiedRequiredAncestor{5064, StrictLevel::True};
inline constexpr ErrorClass UnsatisfiableRequiredAncestor{5065, StrictLevel::True};
// inline constexpr ErrorClass ExperimentalRequiredAncestor{5066, StrictLevel::False};
inline constexpr ErrorClass NonClassSuperclass{5067, StrictLevel::False};
inline constexpr ErrorClass AmbiguousDefinitionError{5068, StrictLevel::False};
inline constexpr ErrorClass MultipleStatementsInSig{5069, StrictLevel::False};
inline constexpr ErrorClass NilableUntyped{5070, StrictLevel::False};
inline constexpr ErrorClass BindNonBlockParameter{5071, StrictLevel::False};
inline constexpr ErrorClass TypeMemberScopeMismatch{5072, StrictLevel::False};
// inline constexpr ErrorClass AbstractClassInstantiated{5073, StrictLevel::True};
inline constexpr ErrorClass HasAttachedClassIncluded{5074, StrictLevel::False};
inline constexpr ErrorClass TypeAliasToTypeMember{5075, StrictLevel::False};
inline constexpr ErrorClass TNilableArity{5076, StrictLevel::False};
inline constexpr ErrorClass UnsupportedLiteralType{5077, StrictLevel::False};
inline constexpr ErrorClass GenericArgumentCountMismatch{5078, StrictLevel::True};
inline constexpr ErrorClass GenericArgumentKeywordArgs{5079, StrictLevel::False};
inline constexpr ErrorClass HasAttachedClassInClass{5080, StrictLevel::False};
inline constexpr ErrorClass HasAttachedClassModuleNotDeclared{5081, StrictLevel::Strict};
inline constexpr ErrorClass UnnamedBlockParameter{5082, StrictLevel::False};
inline constexpr ErrorClass PackageNamespaceMixin{5083, StrictLevel::False};
inline constexpr ErrorClass ModifyingUnpackagedConstant{5084, StrictLevel::False};
inline constexpr ErrorClass InvalidPackageExpression{5085, StrictLevel::False};
} // namespace sorbet::core::errors::Resolver

#endif
