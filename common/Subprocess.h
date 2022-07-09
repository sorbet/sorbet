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

    static std::optional<Subprocess::Result> spawnAndPipeInput(std::string executable, std::string stdinContents);
};

} // namespace sorbet
#endif // SORBET_SUBPROCESS_H
