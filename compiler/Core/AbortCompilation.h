#include "common/Exception.h"

namespace sorbet::compiler {
class AbortCompilation : public sorbet::SorbetException {
public:
    AbortCompilation(const std::string &message) : SorbetException(message){};
    AbortCompilation(const char *message) : SorbetException(message){};
};
} // namespace sorbet::compiler
