#include "common/common.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "main/options/options.h"
#include "main/realmain.h"
int main(int argc, char *argv[]) {
    try {
        sorbet::compiler::ObjectFileEmitter::init();
        auto code = sorbet::realmain::realmain(argc, argv);
        return code;
    } catch (sorbet::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        return 1;
    }
};
