#ifndef SORBET_CORE_ERRORS_CFG_H
#define SORBET_CORE_ERRORS_CFG_H
#include "core/Error.h"

namespace sorbet::core::errors::CFG {
constexpr ErrorClass NoNextScope{6001, StrictLevel::False};
constexpr ErrorClass UndeclaredVariable{6002, StrictLevel::Strict};
// constexpr ErrorClass ReturnExprVoid{6003, StrictLevel::True};
constexpr ErrorClass MalformedTAbsurd{6004, StrictLevel::True};
constexpr ErrorClass MalformedTBind{6005, StrictLevel::False};
constexpr ErrorClass UnknownTypeParameter{6006, StrictLevel::True};
constexpr ErrorClass AbstractClassInstantiated{6007, StrictLevel::True};
} // namespace sorbet::core::errors::CFG
#endif
