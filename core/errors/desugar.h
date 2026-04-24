#ifndef SORBET_CORE_ERRORS_DESUGAR_H
#define SORBET_CORE_ERRORS_DESUGAR_H
#include "core/Error.h"

namespace sorbet::core::errors::Desugar {
inline constexpr ErrorClass InvalidSingletonDef{3001, StrictLevel::True};
inline constexpr ErrorClass IntegerOutOfRange{3002, StrictLevel::True};
inline constexpr ErrorClass UnsupportedNode{3003, StrictLevel::False};
inline constexpr ErrorClass FloatOutOfRange{3004, StrictLevel::True};
inline constexpr ErrorClass NoConstantReassignment{3005, StrictLevel::True};
// inline constexpr ErrorClass SimpleSuperclass{3006, StrictLevel::True};
// inline constexpr ErrorClass UnnamedBlockParameter{3007, StrictLevel::Strict};
inline constexpr ErrorClass UndefUsage{3008, StrictLevel::Strict};
inline constexpr ErrorClass UnsupportedRestArgsDestructure{3009, StrictLevel::True};
inline constexpr ErrorClass CodeInRBI{3010, StrictLevel::False};
inline constexpr ErrorClass DuplicatedHashKeys{3011, StrictLevel::False};
inline constexpr ErrorClass BlockAnonymousRestParam{3012, StrictLevel::False};
} // namespace sorbet::core::errors::Desugar

#endif
