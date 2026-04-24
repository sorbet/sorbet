#ifndef SORBET_CORE_ERRORS_DSL_H
#define SORBET_CORE_ERRORS_DSL_H
#include "core/Error.h"

namespace sorbet::core::errors::Rewriter {
inline constexpr ErrorClass BadAttrArg{3501, StrictLevel::True};
// inline constexpr ErrorClass BadWrapInstance{3502, StrictLevel::True};
inline constexpr ErrorClass PrivateMethodMismatch{3503, StrictLevel::False};
inline constexpr ErrorClass BadAttrType{3504, StrictLevel::True};
inline constexpr ErrorClass BadModuleFunction{3505, StrictLevel::True};
inline constexpr ErrorClass BadTEnumSyntax{3506, StrictLevel::False};
inline constexpr ErrorClass BadTestEach{3507, StrictLevel::True};
inline constexpr ErrorClass PropForeignStrict{3508, StrictLevel::False};
inline constexpr ErrorClass ComputedBySymbol{3509, StrictLevel::False};
inline constexpr ErrorClass InitializeReturnType{3510, StrictLevel::False};
inline constexpr ErrorClass InvalidStructMember{3511, StrictLevel::False};
inline constexpr ErrorClass NilableUntyped{3512, StrictLevel::False};
// moved to namer:
// inline constexpr ErrorClass HasAttachedClassInClass{3513, StrictLevel::False};
inline constexpr ErrorClass ContravariantHasAttachedClass{3514, StrictLevel::False};
inline constexpr ErrorClass DuplicateProp{3515, StrictLevel::True};
inline constexpr ErrorClass VoidAttrReader{3516, StrictLevel::True};
inline constexpr ErrorClass PropBadOverride{3517, StrictLevel::False};

// Let's reserve 3550-3569 for RBS related errors
inline constexpr ErrorClass RBSSyntaxError{3550, StrictLevel::False};
inline constexpr ErrorClass RBSUnsupported{3551, StrictLevel::False};
inline constexpr ErrorClass RBSParameterMismatch{3552, StrictLevel::False};
inline constexpr ErrorClass RBSAssertionError{3553, StrictLevel::False};
inline constexpr ErrorClass RBSUnusedComment{3554, StrictLevel::False};
inline constexpr ErrorClass RBSMultilineMisformatted{3555, StrictLevel::False};
inline constexpr ErrorClass RBSIncorrectParameterKind{3556, StrictLevel::False};
inline constexpr ErrorClass RBSMultipleGenericSignatures{3557, StrictLevel::False};
inline constexpr ErrorClass RBSAbstractMethodNoRaises{3558, StrictLevel::False};

} // namespace sorbet::core::errors::Rewriter
#endif
