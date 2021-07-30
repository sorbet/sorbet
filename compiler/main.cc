#include "common/common.h"
#include "main/options/options.h"
#include "main/realmain.h"

#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"

int main(int argc, char *argv[]) {
    try {
        sorbet::compiler::ObjectFileEmitter::init();
        return sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        return 1;
    }
};
