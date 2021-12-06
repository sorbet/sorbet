#include "payload/payload.h"
#include "common/Timer.h"
#include "core/serialize/serialize.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"

using namespace std;

namespace sorbet::payload {
void createInitialGlobalState(unique_ptr<core::GlobalState> &gs, const realmain::options::Options &options,
                              const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (kvstore) {
        auto maybeGsBytes = kvstore->read(GLOBAL_STATE_KEY);
        if (maybeGsBytes.data != nullptr) {
            Timer timeit(gs->tracer(), "read_global_state.kvstore");
            core::serialize::Serializer::loadGlobalState(*gs, maybeGsBytes.data);
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
    ENFORCE(gs->utf8NamesUsed() < core::GlobalState::PAYLOAD_MAX_UTF8_NAME_COUNT,
            "Payload defined `{}` UTF8 names, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_UTF8_NAME_COUNT` in `GlobalState`.",
            gs->utf8NamesUsed(), core::GlobalState::PAYLOAD_MAX_UTF8_NAME_COUNT);
    ENFORCE(
        gs->constantNamesUsed() < core::GlobalState::PAYLOAD_MAX_CONSTANT_NAME_COUNT,
        "Payload defined `{}` Constant names, which is greater than the expected maximum of `{}`. Consider updating "
        "`PAYLOAD_MAX_CONSTANT_NAME_COUNT` in `GlobalState`.",
        gs->constantNamesUsed(), core::GlobalState::PAYLOAD_MAX_CONSTANT_NAME_COUNT);
    ENFORCE(gs->uniqueNamesUsed() < core::GlobalState::PAYLOAD_MAX_UNIQUE_NAME_COUNT,
            "Payload defined `{}` Unique names, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_UNIQUE_NAME_COUNT` in `GlobalState`.",
            gs->uniqueNamesUsed(), core::GlobalState::PAYLOAD_MAX_UNIQUE_NAME_COUNT);
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
} // namespace sorbet::payload
