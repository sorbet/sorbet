#ifndef SORBET_COMPILER_CORE_ABORT_COMPILATION_H
#define SORBET_COMPILER_CORE_ABORT_COMPILATION_H

#include "common/exception/Exception.h"

namespace sorbet::compiler {
class AbortCompilation : public sorbet::SorbetException {
public:
    AbortCompilation(const std::string &message) : SorbetException(message){};
    AbortCompilation(const char *message) : SorbetException(message){};
};
} // namespace sorbet::compiler

#endif
