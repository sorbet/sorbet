#ifndef SORBET_CORE_ERRORS_CFG_H
#define SORBET_CORE_ERRORS_CFG_H
#include "core/Error.h"

namespace sorbet::core::errors::CFG {
inline constexpr ErrorClass NoNextScope{6001, StrictLevel::False};
inline constexpr ErrorClass UndeclaredVariable{6002, StrictLevel::Strict};
// inline constexpr ErrorClass ReturnExprVoid{6003, StrictLevel::True};
inline constexpr ErrorClass MalformedTAbsurd{6004, StrictLevel::True};
inline constexpr ErrorClass MalformedTBind{6005, StrictLevel::False};
inline constexpr ErrorClass UnknownTypeParameter{6006, StrictLevel::True};
inline constexpr ErrorClass AbstractClassInstantiated{6007, StrictLevel::True};
} // namespace sorbet::core::errors::CFG
#endif
