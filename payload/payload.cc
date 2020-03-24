#include "payload/payload.h"
#include "common/Timer.h"
#include "core/serialize/serialize.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"

using namespace std;

namespace sorbet::payload {

constexpr string_view GLOBAL_STATE_KEY = "GlobalState"sv;

void createInitialGlobalState(unique_ptr<core::GlobalState> &gs, const realmain::options::Options &options,
                              const unique_ptr<const OwnedKeyValueStore> &kvstore) {
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
            gs->kvstoreSessionId = kvstore->sessionId();
            return;
        }
    }
    if (options.noStdlib) {
        gs->initEmpty();
        return;
    }

    const u1 *const nameTablePayload = getNameTablePayload;
    if (nameTablePayload == nullptr) {
        Timer timeit(gs->tracer(), "read_global_state.source");
        sorbet::rbi::polulateRBIsInto(gs);
    } else {
        Timer timeit(gs->tracer(), "read_global_state.binary");
        core::serialize::Serializer::loadGlobalState(*gs, nameTablePayload);
    }
}

bool retainGlobalState(core::GlobalState &gs, const realmain::options::Options &options,
                       const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore && gs.wasModified() && !gs.hadCriticalError()) {
        Timer timeit(gs.tracer(), "write_global_state.kvstore");
        kvstore->write(GLOBAL_STATE_KEY, core::serialize::Serializer::storePayloadAndNameTable(gs));
        return true;
    }
    return false;
}
} // namespace sorbet::payload
