#include "llvm/linker/linker.h"

namespace sorbet::llvm::linker {

void run(std::shared_ptr<spdlog::logger> logger) {
    logger->error("hello world");
}

}
