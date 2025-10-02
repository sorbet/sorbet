#include "payload/payload.h"
#include "common/timers/Timer.h"
#include "core/serialize/serialize.h"
#include "payload/binary/binary.h"
#include "payload/text/text.h"

using namespace std;

namespace sorbet::payload {

void createInitialGlobalState(core::GlobalState &gs, const realmain::options::Options &options,
                              const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (options.cacheSensitiveOptions.noStdlib) {
        gs.initEmpty();
        return;
    }

    if (PAYLOAD_EMPTY) {
        Timer timeit(gs.tracer(), "read_global_state.source");
        sorbet::rbi::populateRBIsInto(gs);
    } else {
        Timer timeit(gs.tracer(), "read_global_state.binary");
        core::serialize::Serializer::loadGlobalState(gs, PAYLOAD_SYMBOL_TABLE, PAYLOAD_NAME_TABLE, PAYLOAD_FILE_TABLE);
    }
    ENFORCE(gs.utf8NamesUsed() < core::GlobalState::PAYLOAD_MAX_UTF8_NAME_COUNT,
            "Payload defined `{}` UTF8 names, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_UTF8_NAME_COUNT` in `GlobalState`.",
            gs.utf8NamesUsed(), core::GlobalState::PAYLOAD_MAX_UTF8_NAME_COUNT);
    ENFORCE(
        gs.constantNamesUsed() < core::GlobalState::PAYLOAD_MAX_CONSTANT_NAME_COUNT,
        "Payload defined `{}` Constant names, which is greater than the expected maximum of `{}`. Consider updating "
        "`PAYLOAD_MAX_CONSTANT_NAME_COUNT` in `GlobalState`.",
        gs.constantNamesUsed(), core::GlobalState::PAYLOAD_MAX_CONSTANT_NAME_COUNT);
    ENFORCE(gs.uniqueNamesUsed() < core::GlobalState::PAYLOAD_MAX_UNIQUE_NAME_COUNT,
            "Payload defined `{}` Unique names, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_UNIQUE_NAME_COUNT` in `GlobalState`.",
            gs.uniqueNamesUsed(), core::GlobalState::PAYLOAD_MAX_UNIQUE_NAME_COUNT);
    ENFORCE(gs.fieldsUsed() < core::GlobalState::PAYLOAD_MAX_FIELD_COUNT,
            "Payload defined `{}` fields, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_FIELD_COUNT` in `GlobalState`.",
            gs.fieldsUsed(), core::GlobalState::PAYLOAD_MAX_FIELD_COUNT);
    ENFORCE(gs.methodsUsed() < core::GlobalState::PAYLOAD_MAX_METHOD_COUNT,
            "Payload defined `{}` methods, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_METHOD_COUNT` in `GlobalState`.",
            gs.methodsUsed(), core::GlobalState::PAYLOAD_MAX_METHOD_COUNT);
    ENFORCE(gs.classAndModulesUsed() < core::GlobalState::PAYLOAD_MAX_CLASS_AND_MODULE_COUNT,
            "Payload defined `{}` classes and modules, which is greater than the expected maximum of `{}`. Consider "
            "updating `PAYLOAD_MAX_CLASS_AND_MODULE_COUNT` in `GlobalState`.",
            gs.classAndModulesUsed(), core::GlobalState::PAYLOAD_MAX_CLASS_AND_MODULE_COUNT);
    ENFORCE(gs.typeMembersUsed() < core::GlobalState::PAYLOAD_MAX_TYPE_MEMBER_COUNT,
            "Payload defined `{}` type members, which is greater than the expected maximum of `{}`. Consider updating "
            "`PAYLOAD_MAX_TYPE_MEMBER_COUNT` in `GlobalState`.",
            gs.typeMembersUsed(), core::GlobalState::PAYLOAD_MAX_TYPE_MEMBER_COUNT);
    ENFORCE(
        gs.typeArgumentsUsed() < core::GlobalState::PAYLOAD_MAX_TYPE_ARGUMENT_COUNT,
        "Payload defined `{}` type arguments, which is greater than the expected maximum of `{}`. Consider updating "
        "`PAYLOAD_MAX_TYPE_ARGUMENT_COUNT` in `GlobalState`.",
        gs.typeArgumentsUsed(), core::GlobalState::PAYLOAD_MAX_TYPE_ARGUMENT_COUNT);

    // We can use the kvstore to read in the cached name table. We read this in after the payload has been initialized,
    // as the cached name table will extend the payload's existing table when the sorbet versions match.
    if (kvstore) {
        auto maybeUUIDBytes = kvstore->read(core::serialize::Serializer::NAME_TABLE_UUID_KEY);
        auto maybeGsBytes = kvstore->read(core::serialize::Serializer::NAME_TABLE_KEY);
        if (maybeUUIDBytes.data != nullptr && maybeGsBytes.data != nullptr) {
            Timer timeit(gs.tracer(), "read_name_table.kvstore");
            core::serialize::Serializer::loadAndOverwriteNameTable(gs, maybeUUIDBytes.data, maybeGsBytes.data);
            if constexpr (debug_mode) {
                for (unsigned int i = 1; i < gs.filesUsed(); i++) {
                    core::FileRef fref(i);

                    // We should only see payload files at this point.
                    ENFORCE(fref.dataAllowingUnsafe(gs).sourceType != core::File::Type::Normal);
                }
            }
        }
    }
}

} // namespace sorbet::payload
