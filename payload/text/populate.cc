#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/Unfreeze.h"
#include "main/pipeline/pipeline.h"
#include "payload/text/text.h"
using namespace std;

namespace sorbet::rbi {
void populateRBIsInto(unique_ptr<core::GlobalState> &gs) {
    gs->initEmpty();
    gs->ensureCleanStrings = true;

    vector<core::FileRef> payloadFiles;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        for (auto &p : rbi::all()) {
            auto file = gs->enterFile(p.first, p.second);
            file.data(*gs).sourceType = core::File::Type::PayloadGeneration;
            payloadFiles.emplace_back(move(file));
        }
    }
    realmain::options::Options emptyOpts;
    unique_ptr<const OwnedKeyValueStore> kvstore;
    auto workers = WorkerPool::create(emptyOpts.threads, gs->tracer());
    auto indexed =
        realmain::pipeline::index(*gs, absl::Span<core::FileRef>(payloadFiles), emptyOpts, *workers, kvstore);

    // We don't run the payload with any packager options, so we can skip pipeline::package()

    // While we want the FoundMethodHashes to end up in the payload, these hashes (including
    // LocalGlobalStateHashes and UsageHash) are not computed until `computeFileHashes` is called in
    // realmain when the `storeState` flag is passed. This means that e.g. sorbet-orig -e
    // '[0].to_set' will typecheck (using text-based payload) but never calculate hashes for the
    // payload files (because neither `--lsp` nor `--store-state` was passed).
    auto foundMethodHashes = nullptr;
    realmain::pipeline::nameAndResolve(gs, move(indexed), emptyOpts, *workers, foundMethodHashes);
    // ^ result is thrown away
    gs->ensureCleanStrings = false;
}

} // namespace sorbet::rbi
