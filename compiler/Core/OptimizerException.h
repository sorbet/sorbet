#include "common/exception/Exception.h"

namespace sorbet::compiler {
// Occasionally we need to throw errors from within LLVM optimization passes,
// where we don't have access to GlobalState and therefore cannot use failCompilation.
// We have this separate class so that the compiler plugin can differentiate this
// exception from AbortCompilation and know that a Sorbet-level error needs to be
// emitted.  The user can therefore have some idea of why Sorbet failed.
class OptimizerException : public sorbet::SorbetException {
public:
    OptimizerException(const std::string &message) : SorbetException(message){};
    OptimizerException(const char *message) : SorbetException(message){};
};
} // namespace sorbet::compiler
