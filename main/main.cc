#include "common/common.h"
#include "main/llvm_main.h"
#include "main/options/options.h"
int main(int argc, char *argv[]) {
    try {
        return sorbet::llvm::realmain::realmain(argc, argv);
    } catch (sorbet::realmain::options::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        return 1;
    }
};
