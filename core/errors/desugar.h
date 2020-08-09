#ifndef SORBET_CORE_ERRORS_DESUGAR_H
#define SORBET_CORE_ERRORS_DESUGAR_H
#include "core/Error.h"

namespace sorbet::core::errors::Desugar {
constexpr ErrorClass InvalidSingletonDef{3001, StrictLevel::True};
constexpr ErrorClass IntegerOutOfRange{3002, StrictLevel::True};
constexpr ErrorClass UnsupportedNode{3003, StrictLevel::False};
constexpr ErrorClass FloatOutOfRange{3004, StrictLevel::True};
constexpr ErrorClass NoConstantReassignment{3005, StrictLevel::True};
// constexpr ErrorClass SimpleSuperclass{3006, StrictLevel::True};
constexpr ErrorClass UnnamedBlockParameter{3007, StrictLevel::Strict};
constexpr ErrorClass UndefUsage{3008, StrictLevel::Strict};
constexpr ErrorClass UnsupportedRestArgsDestructure{3009, StrictLevel::True};
constexpr ErrorClass CodeInRBI{3010, StrictLevel::False};
constexpr ErrorClass DuplicatedHashKeys{3011, StrictLevel::False};
} // namespace sorbet::core::errors::Desugar

#endif
