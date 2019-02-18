#ifndef SORBET_SUBPROCESS_H
#define SORBET_SUBPROCESS_H
#include <optional>
#include <string>

namespace sorbet {

class Subprocess {
    Subprocess() = delete;

public:
    static std::optional<std::string> spawn(std::string executable, std::vector<std::string> arguments);
};

} // namespace sorbet
#endif // SORBET_SUBPROCESS_H
