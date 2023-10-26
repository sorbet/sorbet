// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "absl/strings/match.h"
#include "core/GlobalState.h"
#include "main/autogen/data/definitions.h"
#include "main/autogen/data/msgpack.h"

using namespace std;
namespace sorbet::autogen {

void MsgpackWriter::packName(core::NameRef nm) {
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
    mpack_write_u32(&writer, id);
}

void MsgpackWriter::packNames(vector<core::NameRef> &names) {
    mpack_start_array(&writer, names.size());
    for (auto nm : names) {
        packName(nm);
    }
    mpack_finish_array(&writer);
}

void packString(mpack_writer_t *writer, string_view str) {
    mpack_write_str(writer, str.data(), str.size());
}

void MsgpackWriter::packBool(bool b) {
    if (b) {
        mpack_write_true(&writer);
    } else {
        mpack_write_false(&writer);
    }
}

void MsgpackWriter::packReferenceRef(ReferenceRef ref) {
    if (!ref.exists()) {
        mpack_write_nil(&writer);
    } else {
        mpack_write_u16(&writer, ref.id());
    }
}

void MsgpackWriter::packDefinitionRef(DefinitionRef ref) {
    if (!ref.exists()) {
        mpack_write_nil(&writer);
    } else {
        mpack_write_u16(&writer, ref.id());
    }
}

void MsgpackWriter::packRange(uint32_t begin, uint32_t end) {
    if (version <= 4) {
        mpack_write_u64(&writer, ((uint64_t)begin << 32) | end);
    } else {
        // The above scheme almost certainly implies that the value written
        // will be larger than 2**32 and therefore will take up a full nine
        // bytes of space in the msgpack file.
        //
        // Writing the values as individual u32s means that each can take
        // advantage of being written as a fixuint/u8/u16/u32 in the output;
        // the length of the range will be usually a fixuint or a u8, resulting
        // in even smaller output than writing the endpoint of the range.
        mpack_write_u32(&writer, begin);
        mpack_write_u32(&writer, end - begin);
    }
}

void MsgpackWriter::packDefinition(core::Context ctx, ParsedFile &pf, Definition &def,
                                   const AutogenConfig &autogenCfg) {
    mpack_start_array(&writer, defAttrs[version].size());

    // raw_full_name
    auto raw_full_name = pf.showFullName(ctx, def.id);
    packNames(raw_full_name);

    // type
    auto defType = def.type;
    mpack_write_u8(&writer, static_cast<uint64_t>(defType));

    // defines_behavior
    packBool(def.defines_behavior);
    const auto &filePath = ctx.file.data(ctx).path();
    ENFORCE(!def.defines_behavior || !ctx.file.data(ctx).isRBI() ||
                absl::c_any_of(autogenCfg.behaviorAllowedInRBIsPaths,
                               [&](auto &allowedPath) { return absl::StartsWith(filePath, allowedPath); }),
            "RBI files should never define behavior");

    // isEmpty
    packBool(def.is_empty);

    // parent_ref
    packReferenceRef(def.parent_ref);

    // aliased_ref
    packReferenceRef(def.aliased_ref);

    // defining_ref
    packReferenceRef(def.defining_ref);
    mpack_finish_array(&writer);
}

void MsgpackWriter::packReference(core::Context ctx, ParsedFile &pf, Reference &ref) {
    mpack_start_array(&writer, refAttrs[version].size());

    // scope
    packDefinitionRef(ref.scope.id());

    // name
    packNames(ref.name.nameParts);

    // nesting
    mpack_start_array(&writer, ref.nesting.size());
    for (auto &scope : ref.nesting) {
        packDefinitionRef(scope.id());
    }
    mpack_finish_array(&writer);

    // expression_range
    auto expression_range = ctx.locAt(ref.definitionLoc).position(ctx);
    packRange(expression_range.first.line, expression_range.second.line);
    // expression_pos_range
    packRange(ref.loc.beginPos(), ref.loc.endPos());

    // resolved
    if (ref.resolved.empty()) {
        mpack_write_nil(&writer);
    } else {
        packNames(ref.resolved.nameParts);
    }

    // is_defining_ref
    packBool(ref.is_defining_ref);

    // parent_of
    packDefinitionRef(ref.parent_of);
    mpack_finish_array(&writer);
}

MsgpackWriter::MsgpackWriter(int version)
    : version(assertValidVersion(version)), refAttrs(refAttrMap.at(version)), defAttrs(defAttrMap.at(version)) {}

string MsgpackWriter::pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg) {
    char *body;
    size_t bodySize;
    mpack_writer_init_growable(&writer, &body, &bodySize);
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

    mpack_start_array(&writer, pf.defs.size());
    for (auto &def : pf.defs) {
        packDefinition(ctx, pf, def, autogenCfg);
    }

    mpack_finish_array(&writer);
    mpack_start_array(&writer, pf.refs.size());
    for (auto &ref : pf.refs) {
        packReference(ctx, pf, ref);
    }
    mpack_finish_array(&writer);
    mpack_finish_array(&writer);

    mpack_writer_destroy(&writer);

    // write header
    char *header;
    size_t headerSize;
    mpack_writer_init_growable(&writer, &header, &headerSize);

    const vector<string> &pfAttrs = parsedFileAttrMap.at(version);

    mpack_start_array(&writer, pfAttrs.size());

    int i = -1;
    mpack_start_array(&writer, symbols.size());
    for (auto sym : symbols) {
        ++i;
        auto str = sym.shortName(ctx);
        packString(&writer, str);
    }
    mpack_finish_array(&writer);

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

    // v5 and up record the size of refs and defs to enable fast skipping
    // of the entire data chunk, rather than reading and discarding
    // individual msgpack fields.
    size_t defsAndRefsSize = bodySize - preDefsSize;
    mpack_write_u64(&writer, defsAndRefsSize);

    mpack_write_object_bytes(&writer, body, bodySize);
    MPACK_FREE(body);

    mpack_writer_destroy(&writer);

    auto ret = string(header, headerSize);
    MPACK_FREE(header);

    return ret;
}

string MsgpackWriter::msgpackGlobalHeader(int version, size_t numFiles) {
    string header;

    const vector<string> &pfAttrs = parsedFileAttrMap.at(version);
    const vector<string> &refAttrs = refAttrMap.at(version);
    const vector<string> &defAttrs = defAttrMap.at(version);

    mpack_writer_t writer;
    char *body;
    size_t bodySize;
    mpack_writer_init_growable(&writer, &body, &bodySize);

    mpack_write_u32(&writer, version);

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
            "symbols",
            "typed_level",
            "ref_count",
            "def_count",
            "defs_and_refs_size",
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
