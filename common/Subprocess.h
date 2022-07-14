#ifndef SORBET_SUBPROCESS_H
#define SORBET_SUBPROCESS_H
#include <optional>
#include <string>

namespace sorbet {

class Subprocess {
    Subprocess() = delete;

public:
    typedef struct {
        std::string output;
        int status;
    } Result;

    static std::optional<Subprocess::Result> spawn(std::string executable, std::vector<std::string> arguments,
                                                   std::optional<std::string_view> stdinContents);
};

} // namespace sorbet
#endif // SORBET_SUBPROCESS_H
