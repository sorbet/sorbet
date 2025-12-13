#ifndef SORBET_COMPILER_LINKER_H
#define SORBET_COMPILER_LINKER_H

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace spdlog {
class logger;
}
namespace sorbet::compiler {
class Linker {
public:
    static bool run(spdlog::logger &log, std::vector<std::string> objectFiles, std::string outputFile);
};
} // namespace sorbet::compiler
#endif
