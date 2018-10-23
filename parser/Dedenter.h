#include <string>

namespace sorbet::parser {

// This is defined in a Dedent.h only for testing, and is not a public API
class Dedenter final {
public:
    Dedenter(int level) : dedentLevel(level), spacesToRemove(level) {}

    std::string dedent(std::string_view str);

private:
    unsigned int dedentLevel;
    unsigned int spacesToRemove;
};

} // namespace sorbet::parser
