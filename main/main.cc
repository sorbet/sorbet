#include "absl/debugging/failure_signal_handler.h"
#include "absl/debugging/symbolize.h"
#include "common/common.h"
#include "main/realmain.h"
int main(int argc, char *argv[]) {
    try {
        // Initialize the symbolizer to get a human-readable stack trace
        absl::InitializeSymbolizer(argv[0]);
        absl::FailureSignalHandlerOptions options;
        absl::InstallFailureSignalHandler(options);
        return sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::realmain::options::EarlyReturnWithCode &c) {
        return c.returnCode;
    } catch (sorbet::SRubyException &e) {
        return 1;
    }
};
