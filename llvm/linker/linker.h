#ifndef SORBET_LLVM_LINKER_H
#define SORBET_LLVM_LINKER_H

#include "spdlog/spdlog.h"

namespace sorbet::llvm::linker {
void init();
void run(std::shared_ptr<spdlog::logger> logger);
} // namespace sorbet::llvm::linker
#endif
