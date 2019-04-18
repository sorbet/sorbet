#include "payload/payload.h"
#include "common/Timer.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "main/pipeline/pipeline.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"

using namespace std;

namespace sorbet::payload {

constexpr string_view GLOBAL_STATE_KEY = "GlobalState"sv;

void createInitialGlobalState(unique_ptr<core::GlobalState> &gs, const realmain::options::Options &options,
                              unique_ptr<KeyValueStore> &kvstore) {
    if (kvstore) {
        auto maybeGsBytes = kvstore->read(GLOBAL_STATE_KEY);
        if (maybeGsBytes) {
            Timer timeit(gs->tracer(), "read_global_state.kvstore");
            core::serialize::Serializer::loadGlobalState(*gs, maybeGsBytes);
            for (unsigned int i = 1; i < gs->filesUsed(); i++) {
                core::FileRef fref(i);
                if (fref.dataAllowingUnsafe(*gs).sourceType == core::File::Type::Normal) {
                    gs = core::GlobalState::markFileAsTombStone(move(gs), fref);
                }
            }
            return;
        }
    }
    if (options.noStdlib) {
        gs->initEmpty();
        return;
    }

    const u1 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        gs->initEmpty();
        Timer timeit(gs->tracer(), "read_global_state.source");

        vector<core::FileRef> payloadFiles;
        {
            core::UnfreezeFileTable fileTableAccess(*gs);
            for (auto &p : rbi::all()) {
                auto file = gs->enterFile(p.first, p.second);
                file.data(*gs).sourceType = core::File::PayloadGeneration;
                payloadFiles.emplace_back(move(file));
            }
        }
        realmain::options::Options emptyOpts;
        WorkerPool workers(emptyOpts.threads, gs->tracer());
        auto indexed = realmain::pipeline::index(gs, payloadFiles, emptyOpts, workers, kvstore);
        realmain::pipeline::resolve(*gs, move(indexed), emptyOpts); // result is thrown away
    } else {
        Timer timeit(gs->tracer(), "read_global_state.binary");
        core::serialize::Serializer::loadGlobalState(*gs, nameTablePayload);
    }
}

void retainGlobalState(unique_ptr<core::GlobalState> &gs, const realmain::options::Options &options,
                       unique_ptr<KeyValueStore> &kvstore) {
    if (!options.storeState.empty()) {
        auto files = gs->getFiles();
        for (const auto &f : files) {
            if (f && f->sourceType != core::File::Type::TombStone && !f->getDefinitionHash().has_value()) {
                f->setDefinitionHash(realmain::pipeline::computeFileHash(f, gs->tracer()));
            }
        }
    }

    if (kvstore && gs->wasModified() && !gs->hadCriticalError()) {
        Timer timeit(gs->tracer(), "write_global_state.kvstore");
        kvstore->write(GLOBAL_STATE_KEY, core::serialize::Serializer::storePayloadAndNameTable(*gs));
        KeyValueStore::commit(move(kvstore));
    }
}
} // namespace sorbet::payload
