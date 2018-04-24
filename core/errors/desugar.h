#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Desugar {
constexpr ErrorClass InvalidSingletonDef{3001, StrictLevel::Typed};
constexpr ErrorClass IntegerOutOfRange{3002, StrictLevel::Typed};
constexpr ErrorClass UnsupportedNode{3003, StrictLevel::Stripe};
constexpr ErrorClass FloatOutOfRange{3004, StrictLevel::Typed};
} // namespace Desugar
} // namespace errors
} // namespace core
} // namespace ruby_typer
