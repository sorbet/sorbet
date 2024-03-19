#include "common/common.h"
#include "main/options/options.h"
#include "main/realmain.h"
#include <signal.h>
int main(int argc, char *argv[]) {
    try {
        return sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SorbetException &e) {
        fprintf(stderr, "caught %s: %s\n", typeid(e).name(), e.what());

        // Forcibly exit with a segfault signal for uncaught exceptions, which makes it easier to
        // use `catchsegv.sh` to report these errors.
        kill(0, SIGSEGV);
    }
};
