#include <string>

namespace sorbet {
namespace parser {

// This is defined in a Dedent.h only for testing, and is not a public API
class Dedenter final {
public:
    Dedenter(int level) : dedent_level(level), spaces_to_remove(level) {}

    std::string dedent(const std::string &str);

private:
    unsigned int dedent_level;
    unsigned int spaces_to_remove;
};

} // namespace parser
} // namespace sorbet
