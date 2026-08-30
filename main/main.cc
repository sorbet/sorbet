#include "common/common.h"
#include "main/options/options.h"
#include "main/realmain.h"

#ifdef SORBET_MIMALLOC
#include "mimalloc.h"

namespace {
// A batch typecheck of a large codebase is a short-lived process that allocates tens of gigabytes and frees most of
// it only at exit. mimalloc's default is to return memory the process has freed to the OS after a short delay, which
// suits the long-lived language server. In a batch run the process frees and re-allocates memory of the same shape
// throughout, so each such purge is a madvise plus a set of fresh page faults for nothing: never purge. (mimalloc
// already backs its arenas with transparent huge pages by default; that needs no setting here.)
//
// Measured on a 600k-file codebase with 16 threads: about 2-3% less wall and CPU, peak RSS unchanged.
void tuneAllocatorForBatchRun() {
    mi_option_set(mi_option_purge_delay, -1);
}
} // namespace
#endif

int main(int argc, char *argv[]) {
#ifdef SORBET_MIMALLOC
    sorbet::realmain::batchAllocatorHook = tuneAllocatorForBatchRun;
#endif
    try {
        return sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::EarlyReturnWithCode &c) {
        return c.returnCode;
    }
};
