#include "main/realmain.h"

namespace sorbet::llvm::main {

int main(int argc, char *argv[]) {
    auto code = sorbet::realmain::realmain(argc, argv);
    printf("hello world\n");
    return code;
}

}
