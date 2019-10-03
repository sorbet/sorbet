#include "common/common.h"
#include "compiler/object_file_emitter/object_file_emitter.h"
#include "main/options/options.h"
#include "main/realmain.h"
int main(int argc, char *argv[]) {
    try {
        sorbet::compiler::ObjectFileEmitter::init();
        auto code = sorbet::realmain::realmain(argc, argv);
        return code;
    } catch (sorbet::realmain::options::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        return 1;
    }
};
