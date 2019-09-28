#include "main/realmain.h"

namespace sorbet::llvm::realmain {

int realmain(int argc, char *argv[]) {
    auto code = sorbet::realmain::realmain(argc, argv);
    sorbet::realmain::logger->error("hello world");
    return code;
}

} // namespace sorbet::llvm::main
