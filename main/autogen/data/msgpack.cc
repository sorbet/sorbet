// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "absl/strings/match.h"
#include "core/GlobalState.h"
#include "main/autogen/data/definitions.h"
#include "main/autogen/data/msgpack.h"

using namespace std;
namespace sorbet::autogen {

void MsgpackWriter::packName(mpack_writer_t *writer, core::NameRef nm) {
    uint32_t id;
    typename decltype(symbolIds)::value_type v{nm, 0};
    auto [it, inserted] = symbolIds.insert(v);
    if (inserted) {
        id = symbols.size();
        symbols.emplace_back(nm);
        it->second = id;
    } else {
        id = it->second;
    }
    mpack_write_u32(writer, id);
}

void MsgpackWriter::packNames(mpack_writer_t *writer, vector<core::NameRef> &names) {
    mpack_start_array(writer, names.size());
    for (auto nm : names) {
        packName(writer, nm);
    }
    mpack_finish_array(writer);
}

void packString(mpack_writer_t *writer, string_view str) {
    mpack_write_str(writer, str.data(), str.size());
}

void MsgpackWriter::packBool(mpack_writer_t *writer, bool b) {
    if (b) {
        mpack_write_true(writer);
    } else {
        mpack_write_false(writer);
    }
}

void MsgpackWriter::packReferenceRef(mpack_writer_t *writer, ReferenceRef ref) {
    if (!ref.exists()) {
        mpack_write_nil(writer);
    } else {
        mpack_write_u16(writer, ref.id());
    }
}

void MsgpackWriter::packDefinitionRef(mpack_writer_t *writer, DefinitionRef ref) {
    if (!ref.exists()) {
        mpack_write_nil(writer);
    } else {
        mpack_write_u16(writer, ref.id());
    }
}

void MsgpackWriter::packRange(mpack_writer_t *writer, uint32_t begin, uint32_t end) {
    if (version <= 4) {
        mpack_write_u64(writer, ((uint64_t)begin << 32) | end);
    } else {
        // The above scheme almost certainly implies that the value written
        // will be larger than 2**32 and therefore will take up a full nine
        // bytes of space in the msgpack file.
        //
        // Writing the values as individual u32s means that each can take
        // advantage of being written as a fixuint/u8/u16/u32 in the output;
        // the length of the range will be usually a fixuint or a u8, resulting
        // in even smaller output than writing the endpoint of the range.
        mpack_write_u32(writer, begin);
        mpack_write_u32(writer, end - begin);
    }
}

void MsgpackWriter::packDefinition(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Definition &def,
                                   const AutogenConfig &autogenCfg) {
    mpack_start_array(writer, defAttrs[version].size());

    // raw_full_name
    auto raw_full_name = pf.showFullName(ctx, def.id);
    packNames(writer, raw_full_name);

    // type
    auto defType = def.type;
    mpack_write_u8(writer, static_cast<uint64_t>(defType));

    // defines_behavior
    packBool(writer, def.defines_behavior);
    const auto &filePath = ctx.file.data(ctx).path();
    ENFORCE(!def.defines_behavior || !ctx.file.data(ctx).isRBI() ||
                absl::c_any_of(autogenCfg.behaviorAllowedInRBIsPaths,
                               [&](auto &allowedPath) { return absl::StartsWith(filePath, allowedPath); }),
            "RBI files should never define behavior");

    // isEmpty
    packBool(writer, def.is_empty);

    // parent_ref
    packReferenceRef(writer, def.parent_ref);

    // aliased_ref
    packReferenceRef(writer, def.aliased_ref);

    // defining_ref
    packReferenceRef(writer, def.defining_ref);
    mpack_finish_array(writer);
}

void MsgpackWriter::packReference(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Reference &ref) {
    mpack_start_array(writer, refAttrs[version].size());

    // scope
    packDefinitionRef(writer, ref.scope.id());

    // name
    packNames(writer, ref.name.nameParts);

    // nesting
    if (version >= 6) {
        mpack_write_u32(writer, ref.nestingId);
    } else {
        auto &nesting = pf.nestings[ref.nestingId];
        mpack_start_array(writer, nesting.size());
        for (auto &scope : nesting) {
            packDefinitionRef(writer, scope.id());
        }
        mpack_finish_array(writer);
    }

    // expression_range
    auto expression_range = ctx.locAt(ref.definitionLoc).position(ctx);
    packRange(writer, expression_range.first.line, expression_range.second.line);
    // expression_pos_range
    packRange(writer, ref.loc.beginPos(), ref.loc.endPos());

    // resolved
    if (ref.resolved.empty()) {
        mpack_write_nil(writer);
    } else if (version >= 6 && absl::c_equal(ref.name.nameParts, ref.resolved.nameParts)) {
        mpack_write_true(writer);
    } else {
        packNames(writer, ref.resolved.nameParts);
    }

    // is_defining_ref
    packBool(writer, ref.is_defining_ref);

    // parent_of
    packDefinitionRef(writer, ref.parent_of);

    mpack_finish_array(writer);
}

MsgpackWriter::MsgpackWriter(int version)
    : version(assertValidVersion(version)), refAttrs(refAttrMap.at(version)), defAttrs(defAttrMap.at(version)),
      pfAttrs(parsedFileAttrMap.at(version)) {}

void writeSymbols(core::Context ctx, mpack_writer_t *writer, const vector<core::NameRef> &symbols) {
    mpack_start_array(writer, symbols.size());
    for (auto sym : symbols) {
        auto str = sym.shortName(ctx);
        packString(writer, str);
    }
    mpack_finish_array(writer);
}

string MsgpackWriter::pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg) {
    char *body;
    size_t bodySize;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &body, &bodySize);

    bool symbolsInBody = version >= 6;

    mpack_start_array(&writer, 6);

    mpack_write_true(&writer); // did_resolution
    packString(&writer, ctx.state.getPrintablePath(pf.path));
    mpack_write_u32(&writer, pf.cksum);

    // requires
    mpack_start_array(&writer, pf.requireStatements.size());
    for (auto nm : pf.requireStatements) {
        packString(&writer, nm.show(ctx));
    }
    mpack_finish_array(&writer);

    size_t preDefsSize = mpack_writer_buffer_used(&writer);

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
        mpack_start_array(&temporaryWriter, pf.defs.size());
        for (auto &def : pf.defs) {
            packDefinition(&temporaryWriter, ctx, pf, def, autogenCfg);
        }
        mpack_finish_array(&temporaryWriter);

        if (version >= 6) {
            mpack_start_array(&temporaryWriter, pf.nestings.size());
            for (auto &nesting : pf.nestings) {
                mpack_start_array(&temporaryWriter, nesting.size());
                for (auto &scope : nesting) {
                    packDefinitionRef(&temporaryWriter, scope.id());
                }
                mpack_finish_array(&temporaryWriter);
            }
            mpack_finish_array(&temporaryWriter);
        }

        mpack_start_array(&temporaryWriter, pf.refs.size());
        for (auto &ref : pf.refs) {
            packReference(&temporaryWriter, ctx, pf, ref);
        }
        mpack_finish_array(&temporaryWriter);
    }

    if (symbolsInBody) {
        writeSymbols(ctx, &writer, symbols);
    }

    mpack_writer_destroy(&temporaryWriter);
    mpack_write_object_bytes(&writer, temporary, temporarySize);
    MPACK_FREE(temporary);

    mpack_finish_array(&writer);

    mpack_writer_destroy(&writer);

    // write header
    char *header;
    size_t headerSize;
    mpack_writer_init_growable(&writer, &header, &headerSize);

    mpack_start_array(&writer, pfAttrs.size());

    if (!symbolsInBody) {
        writeSymbols(ctx, &writer, symbols);
    }

    if (version >= 6) {
        uint32_t value = 0;

        switch (pf.tree.file.data(ctx).strictLevel) {
            case sorbet::core::StrictLevel::Ignore:
                value = 1;
                break;
            case sorbet::core::StrictLevel::False:
                value = 2;
                break;
            case sorbet::core::StrictLevel::True:
                value = 3;
                break;
            case sorbet::core::StrictLevel::Strict:
                value = 4;
                break;
            case sorbet::core::StrictLevel::Strong:
                value = 5;
                break;
            default:
                // Default value already set at 0.
                break;
        }
        mpack_write_u32(&writer, value);
    }

    mpack_write_u32(&writer, pf.refs.size());
    mpack_write_u32(&writer, pf.defs.size());

    if (symbolsInBody) {
        mpack_write_u32(&writer, symbols.size());
    }

    // v5 and up record the size of the parsed file's body to enable fast skipping
    // of the entire data chunk, rather than reading and discarding
    // individual msgpack fields.
    size_t fieldsSize = bodySize - preDefsSize;
    mpack_write_u64(&writer, fieldsSize);

    mpack_write_object_bytes(&writer, body, bodySize);
    MPACK_FREE(body);

    mpack_writer_destroy(&writer);

    auto ret = string(header, headerSize);
    MPACK_FREE(header);

    return ret;
}

string buildGlobalHeader(int version, int serializedVersion, size_t numFiles, const std::vector<std::string> &refAttrs,
                         const std::vector<std::string> &defAttrs, const std::vector<std::string> &pfAttrs) {
    string header;

    mpack_writer_t writer;
    char *body;
    size_t bodySize;
    mpack_writer_init_growable(&writer, &body, &bodySize);

    mpack_write_u32(&writer, serializedVersion);

    mpack_start_array(&writer, pfAttrs.size());
    for (const auto &attr : pfAttrs) {
        packString(&writer, attr);
    }

    mpack_start_array(&writer, refAttrs.size());
    for (const auto &attr : refAttrs) {
        packString(&writer, attr);
    }
    mpack_finish_array(&writer);

    mpack_start_array(&writer, defAttrs.size());
    for (const auto &attr : defAttrs) {
        packString(&writer, attr);
    }
    mpack_finish_array(&writer);

    mpack_write_u64(&writer, numFiles);

    mpack_writer_destroy(&writer);

    header = string(body, bodySize);
    MPACK_FREE(body);

    return header;
}

string MsgpackWriter::msgpackGlobalHeader(int version, size_t numFiles) {
    const vector<string> &pfAttrs = parsedFileAttrMap.at(version);
    const vector<string> &refAttrs = refAttrMap.at(version);
    const vector<string> &defAttrs = defAttrMap.at(version);

    return buildGlobalHeader(version, version, numFiles, refAttrs, defAttrs, pfAttrs);
}

const map<int, vector<string>> MsgpackWriter::parsedFileAttrMap{
    {
        5,
        {
            "symbols",
            "ref_count",
            "def_count",
            "defs_and_refs_size",
        },
    },
    {
        6,
        {
            "typed_level",
            "ref_count",
            "def_count",
            "sym_count",
            "body_size",
        },
    },
};

const map<int, vector<string>> MsgpackWriter::refAttrMap{
    {5,
     {
         "scope",
         "name",
         "nesting",
         "expr_range_start",
         "expr_range_len",
         "expr_pos_range_start",
         "expr_pos_range_len",
         "resolved",
         "is_defining_ref",
         "parent_of",
     }},
    {6,
     {
         "scope",
         "name",
         "nesting",
         "expr_range_start",
         "expr_range_len",
         "expr_pos_range_start",
         "expr_pos_range_len",
         "resolved",
         "is_defining_ref",
         "parent_of",
     }},
};

const map<int, vector<string>> MsgpackWriter::defAttrMap{
    {
        5,
        {
            "raw_full_name",
            "type",
            "defines_behavior",
            "is_empty",
            "parent_ref",
            "aliased_ref",
            "defining_ref",
        },
    },
    {
        6,
        {
            "raw_full_name",
            "type",
            "defines_behavior",
            "is_empty",
            "parent_ref",
            "aliased_ref",
            "defining_ref",
        },
    },
};

} // namespace sorbet::autogen
