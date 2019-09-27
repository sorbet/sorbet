#include "main/realmain.h"

namespace sorbet::llvm::main {

int main(int argc, char *argv[]) {
    auto code = sorbet::realmain::realmain(argc, argv);
    realmain::logger->error("hello world");
    return code;
}

} // namespace sorbet::llvm::main
