#include "common/common.h"
#include "llvm/linker/linker.h"
#include "main/options/options.h"
#include "main/realmain.h"
int main(int argc, char *argv[]) {
    try {
        auto code = sorbet::realmain::realmain(argc, argv);
        sorbet::llvm::linker::run(sorbet::realmain::logger);
        return code;
    } catch (sorbet::realmain::options::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        return 1;
    }
};
