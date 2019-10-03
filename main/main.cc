#include "common/common.h"
#include "main/options/options.h"
#include "main/realmain.h"
#include "llvm/linker/linker.h"
int main(int argc, char *argv[]) {
    try {
        sorbet::compiler::Linker::init();
        auto code = sorbet::realmain::realmain(argc, argv);
        return code;
    } catch (sorbet::realmain::options::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        return 1;
    }
};
