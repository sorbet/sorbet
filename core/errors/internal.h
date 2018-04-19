#include "core/Errors.h"

namespace ruby_typer {
namespace core {
namespace errors {
namespace Internal {
constexpr ErrorClass InternalError = {1001, core::StrictLevel::Ruby};
}
} // namespace errors
} // namespace core
} // namespace ruby_typer
