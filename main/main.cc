#include "common/common.h"
#include "main/realmain.h"
int main(int argc, char *argv[]) {
    try {
        return sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::realmain::options::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SRubyException &e) {
        return 1;
    }
};
