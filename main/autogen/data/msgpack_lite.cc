// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "absl/strings/match.h"
#include "core/GlobalState.h"
#include "main/autogen/data/definitions.h"
#include "main/autogen/data/msgpack.h"

using namespace std;
namespace sorbet::autogen {

void MsgpackWriterLite::packDefinition(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Definition &def,
                                       const AutogenConfig &autogenCfg) {
    optional<MsgpackArray> defArray;

    if (version <= 6) {
        defArray.emplace(writer, defAttrs.size());
    }

    // raw_full_name
    auto raw_full_name = pf.showFullName(ctx, def.id);
    packNames(writer, raw_full_name);

    // type
    if (version >= 7) {
        auto defType = def.type;
        mpack_write_u8(writer, static_cast<uint64_t>(defType));
    }

    // defines_behavior
    packBool(writer, def.defines_behavior);
    const auto &filePath = ctx.file.data(ctx).path();
    ENFORCE(!def.defines_behavior || !ctx.file.data(ctx).isRBI() ||
                absl::c_any_of(autogenCfg.behaviorAllowedInRBIsPaths,
                               [&](auto &allowedPath) { return absl::StartsWith(filePath, allowedPath); }),
            "RBI files should never define behavior");

    if (version >= 7) {
        // parent_ref
        packReferenceRef(writer, def.parent_ref);

        // defining_ref
        packReferenceRef(writer, def.defining_ref);
    }
}

void MsgpackWriterLite::packReference(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Reference &ref) {
    optional<MsgpackArray> refArray;

    if (version <= 6) {
        refArray.emplace(writer, refAttrs.size());
    }

    // scope
    if (version >= 7) {
        packDefinitionRef(writer, ref.scope.id());
    }

    // name
    packNames(writer, ref.name.nameParts);

    // resolved
    if (ref.resolved.empty()) {
        mpack_write_nil(writer);
    } else if (absl::c_equal(ref.name.nameParts, ref.resolved.nameParts)) {
        mpack_write_true(writer);
    } else {
        packNames(writer, ref.resolved.nameParts);
    }

    // is_defining_ref
    if (version >= 7) {
        packBool(writer, ref.is_defining_ref);
    }
}

MsgpackWriterLite::MsgpackWriterLite(int version)
    : MsgpackWriterBase(assertValidVersion(version), refAttrMap.at(version), defAttrMap.at(version),
                        parsedFileAttrMap.at(version)) {}

string MsgpackWriterLite::pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg) {
    char *body;
    size_t bodySize;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &body, &bodySize);

    size_t preDefsSize;
    {
        MsgpackArray info(&writer, 3);

        // path: 1
        packString(&writer, ctx.state.getPrintablePath(pf.path));

        preDefsSize = mpack_writer_buffer_used(&writer);

        // This is a little awkward.  We want to write the symbols used by
        // defs and refs here, but the symbols hash isn't populated until after
        // we've written the defs and refs.  So we're going to redirect
        // everything into a temporary buffer, write the now-populated
        // symbols, then write the temporary buffer as raw bytes.
        char *temporary;
        size_t temporarySize;
        mpack_writer_t temporaryWriter;
        mpack_writer_init_growable(&temporaryWriter, &temporary, &temporarySize);
        {
            {
                MsgpackArray defs(&temporaryWriter, pf.defs.size());
                for (auto &def : pf.defs) {
                    packDefinition(&temporaryWriter, ctx, pf, def, autogenCfg);
                }
            }

            {
                MsgpackArray refs(&temporaryWriter, pf.refs.size());
                for (auto &ref : pf.refs) {
                    packReference(&temporaryWriter, ctx, pf, ref);
                }
            }
        }

        // symbols: 2
        writeSymbols(ctx, &writer, symbols);

        mpack_writer_destroy(&temporaryWriter);

        // defs / refs: 3
        mpack_write_object_bytes(&writer, temporary, temporarySize);
        MPACK_FREE(temporary);
    }

    mpack_writer_destroy(&writer);

    // write header
    char *header;
    size_t headerSize;
    mpack_writer_init_growable(&writer, &header, &headerSize);

    {
        MsgpackArray attributes(&writer, pfAttrs.size());

        if (version >= 7) {
            uint32_t value = strictLevelToInt(pf.file.data(ctx).strictLevel);
            mpack_write_u32(&writer, value);
        }

        mpack_write_u32(&writer, pf.refs.size());
        mpack_write_u32(&writer, pf.defs.size());
        mpack_write_u32(&writer, symbols.size());

        // v5 and up record the size of the parsed file's body to enable fast skipping
        // of the entire data chunk, rather than reading and discarding
        // individual msgpack fields.
        size_t fieldsSize = bodySize - preDefsSize;
        mpack_write_u64(&writer, fieldsSize);
    }

    mpack_write_object_bytes(&writer, body, bodySize);
    MPACK_FREE(body);

    mpack_writer_destroy(&writer);

    auto ret = string(header, headerSize);
    MPACK_FREE(header);

    return ret;
}

// TODO (aadi-stripe, 6/20/2024): Ideally this static function does need to be repeated in the
// MsgpackWriterLite class, consider refactoring by using templates.
string MsgpackWriterLite::msgpackGlobalHeader(int version, size_t numFiles) {
    const vector<string> &pfAttrs = parsedFileAttrMap.at(version);
    const vector<string> &refAttrs = refAttrMap.at(version);
    const vector<string> &defAttrs = defAttrMap.at(version);

    // set a high-bit (bit 8) on version to 1, to indicate "lite" msgpack mode.
    int serializedVersion = version + 128;
    return buildGlobalHeader(version, serializedVersion, numFiles, refAttrs, defAttrs, pfAttrs);
}

const map<int, vector<string>> MsgpackWriterLite::parsedFileAttrMap{{
                                                                        6,
                                                                        {
                                                                            "ref_count",
                                                                            "def_count",
                                                                            "sym_count",
                                                                            "body_size",
                                                                        },
                                                                    },
                                                                    {
                                                                        7,
                                                                        {
                                                                            "typed_level",
                                                                            "ref_count",
                                                                            "def_count",
                                                                            "sym_count",
                                                                            "body_size",
                                                                        },
                                                                    }};

const map<int, vector<string>> MsgpackWriterLite::refAttrMap{{
                                                                 6,
                                                                 {
                                                                     "name",
                                                                     "resolved",
                                                                 },
                                                             },
                                                             {
                                                                 7,
                                                                 {
                                                                     "scope",
                                                                     "name",
                                                                     "resolved",
                                                                     "is_defining_ref",
                                                                 },
                                                             }};

const map<int, vector<string>> MsgpackWriterLite::defAttrMap{
    {
        6,
        {
            "raw_full_name",
            "defines_behavior",
        },
    },
    {
        7,
        {
            "raw_full_name",
            "type",
            "defines_behavior",
            "parent_ref",
            "defining_ref",
        },
    },
};

} // namespace sorbet::autogen
