#include "payload/payload.h"
#include "common/Random.h"
#include "common/Timer.h"
#include "core/serialize/serialize.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"

using namespace std;

namespace sorbet::payload {

namespace {
constexpr string_view GLOBAL_STATE_KEY = "GlobalState"sv;
}

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
        sorbet::rbi::populateRBIsInto(gs);
    } else {
        Timer timeit(gs->tracer(), "read_global_state.binary");
        core::serialize::Serializer::loadGlobalState(*gs, nameTablePayload);
    }
    ENFORCE(gs->namesUsed() < core::GlobalState::PAYLOAD_MAX_NAME_COUNT,
            "Payload defined `{}` names, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_NAME_COUNT` in `GlobalState`.",
            gs->namesUsed(), core::GlobalState::PAYLOAD_MAX_NAME_COUNT);
    ENFORCE(gs->fieldsUsed() < core::GlobalState::PAYLOAD_MAX_FIELD_COUNT,
            "Payload defined `{}` fields, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_FIELD_COUNT` in `GlobalState`.",
            gs->fieldsUsed(), core::GlobalState::PAYLOAD_MAX_FIELD_COUNT);
    ENFORCE(gs->methodsUsed() < core::GlobalState::PAYLOAD_MAX_METHOD_COUNT,
            "Payload defined `{}` methods, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_METHOD_COUNT` in `GlobalState`.",
            gs->methodsUsed(), core::GlobalState::PAYLOAD_MAX_METHOD_COUNT);
    ENFORCE(gs->classAndModulesUsed() < core::GlobalState::PAYLOAD_MAX_CLASS_AND_MODULE_COUNT,
            "Payload defined `{}` classes and modules, which is greater than the expected maximum of `{}`. Consider "
            "updating `PAYLOAD_MAX_CLASS_AND_MODULE_COUNT` in `GlobalState`.",
            gs->classAndModulesUsed(), core::GlobalState::PAYLOAD_MAX_CLASS_AND_MODULE_COUNT);
    ENFORCE(gs->typeMembersUsed() < core::GlobalState::PAYLOAD_MAX_TYPE_MEMBER_COUNT,
            "Payload defined `{}` type members, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_TYPE_MEMBER_COUNT` in `GlobalState`.",
            gs->typeMembersUsed(), core::GlobalState::PAYLOAD_MAX_TYPE_MEMBER_COUNT);
    ENFORCE(
        gs->typeArgumentsUsed() < core::GlobalState::PAYLOAD_MAX_TYPE_ARGUMENT_COUNT,
        "Payload defined `{}` type arguments, which is greater than the expected maximum of `{}`. Consider updating "
        "`PAYLOAD_MAX_TYPE_ARGUMENT_COUNT` in `GlobalState`.",
        gs->typeArgumentsUsed(), core::GlobalState::PAYLOAD_MAX_TYPE_ARGUMENT_COUNT);
}

namespace {
bool kvstoreUnchangedSinceGsCreation(const core::GlobalState &gs, const u1 *maybeGsBytes) {
    const bool storedUidMatches =
        maybeGsBytes && gs.kvstoreUuid == core::serialize::Serializer::loadGlobalStateUUID(gs, maybeGsBytes);
    const bool noPreviouslyStoredUuid = !maybeGsBytes && gs.kvstoreUuid == 0;
    return storedUidMatches || noPreviouslyStoredUuid;
}
} // namespace

bool kvstoreUnchangedSinceGsCreation(const core::GlobalState &gs, const unique_ptr<OwnedKeyValueStore> &kvstore) {
    return kvstoreUnchangedSinceGsCreation(gs, kvstore->read(GLOBAL_STATE_KEY));
}

bool retainGlobalState(core::GlobalState &gs, const realmain::options::Options &options,
                       const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore && gs.wasModified() && !gs.hadCriticalError()) {
        auto maybeGsBytes = kvstore->read(GLOBAL_STATE_KEY);
        // Verify that no other GlobalState was written to kvstore between when we read GlobalState and wrote it
        // into the databaase.
        if (kvstoreUnchangedSinceGsCreation(gs, maybeGsBytes)) {
            Timer timeit(gs.tracer(), "write_global_state.kvstore");
            // Generate a new UUID, since this GS has changed since it was read.
            gs.kvstoreUuid = Random::uniformU4();
            kvstore->write(GLOBAL_STATE_KEY, core::serialize::Serializer::storePayloadAndNameTable(gs));
            return true;
        }
    }
    return false;
}
} // namespace sorbet::payload
