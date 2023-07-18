#ifndef SORBET_CORE_ERRORS_DSL_H
#define SORBET_CORE_ERRORS_DSL_H
#include "core/Error.h"

namespace sorbet::core::errors::Rewriter {
constexpr ErrorClass BadAttrArg{3501, StrictLevel::True};
constexpr ErrorClass BadWrapInstance{3502, StrictLevel::True};
constexpr ErrorClass PrivateMethodMismatch{3503, StrictLevel::False};
constexpr ErrorClass BadAttrType{3504, StrictLevel::True};
constexpr ErrorClass BadModuleFunction{3505, StrictLevel::True};
constexpr ErrorClass BadTEnumSyntax{3506, StrictLevel::False};
constexpr ErrorClass BadTestEach{3507, StrictLevel::True};
constexpr ErrorClass PropForeignStrict{3508, StrictLevel::False};
constexpr ErrorClass ComputedBySymbol{3509, StrictLevel::False};
constexpr ErrorClass InitializeReturnType{3510, StrictLevel::False};
constexpr ErrorClass InvalidStructMember{3511, StrictLevel::False};
constexpr ErrorClass NilableUntyped{3512, StrictLevel::False};
// moved to namer:
// constexpr ErrorClass HasAttachedClassInClass{3513, StrictLevel::False};
constexpr ErrorClass ContravariantHasAttachedClass{3514, StrictLevel::False};
} // namespace sorbet::core::errors::Rewriter
#endif
