#include "common/common.h"
#include "main/options/options.h"
#include "main/realmain.h"
int main(int argc, char *argv[]) {
    try {
        return sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::EarlyReturnWithCode &c) {
        return c.returnCode;
    }
};
