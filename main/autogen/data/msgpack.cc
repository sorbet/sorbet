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

void MsgpackWriter::packString(string_view str) {
    mpack_write_str(&writer, str.data(), str.size());
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
    mpack_write_u64(&writer, ((uint64_t)begin << 32) | end);
}

void MsgpackWriter::packDefinition(core::Context ctx, ParsedFile &pf, Definition &def,
                                   const AutogenConfig &autogenCfg) {
    mpack_start_array(&writer, defAttrs[version].size());

    // raw_full_name
    auto raw_full_name = pf.showFullName(ctx, def.id);
    packNames(raw_full_name);

    // type
    auto defType = def.type;
    if (version <= 2 && defType == Definition::Type::TypeAlias) {
        defType = Definition::Type::Casgn;
    }
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

// symbols[0..(typeCount-1)] are reserved for the Type aliases
MsgpackWriter::MsgpackWriter(int version)
    : version(assertValidVersion(version)), refAttrs(refAttrMap.at(version)), defAttrs(defAttrMap.at(version)),
      symbols(typeCount.at(version)) {}

string MsgpackWriter::pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg) {
    char *body;
    size_t bodySize;
    mpack_writer_init_growable(&writer, &body, &bodySize);
    mpack_start_array(&writer, 6);

    mpack_write_true(&writer); // did_resolution
    packString(ctx.state.getPrintablePath(pf.path));
    mpack_write_u32(&writer, pf.cksum);

    // requires
    mpack_start_array(&writer, pf.requireStatements.size());
    for (auto nm : pf.requireStatements) {
        packString(nm.show(ctx));
    }
    mpack_finish_array(&writer);

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

    mpack_start_map(&writer, 5);

    packString("symbols");
    int i = -1;
    int numTypes = typeCount.at(version);
    mpack_start_array(&writer, symbols.size());
    for (auto sym : symbols) {
        ++i;
        string_view str;
        if (i < numTypes) {
            switch ((Definition::Type)i) {
                case Definition::Type::Module:
                    str = "module"sv;
                    break;
                case Definition::Type::Class:
                    str = "class"sv;
                    break;
                case Definition::Type::Casgn:
                    str = "casgn"sv;
                    break;
                case Definition::Type::Alias:
                    str = "alias"sv;
                    break;
                case Definition::Type::TypeAlias:
                    str = "typealias"sv;
                    break;
                default: {
                    // shouldn't happen
                    auto v = sym.shortName(ctx);
                    static_assert(std::is_same_v<decltype(v), string_view>, "shortName doesn't return the right thing");
                    str = v;
                    break;
                }
            }
        } else {
            auto v = sym.shortName(ctx);
            static_assert(std::is_same_v<decltype(v), string_view>, "shortName doesn't return the right thing");
            str = v;
        }

        packString(str);
    }
    mpack_finish_array(&writer);

    packString("ref_count");
    mpack_write_u32(&writer, pf.refs.size());
    packString("def_count");
    mpack_write_u32(&writer, pf.defs.size());

    packString("ref_attrs");
    mpack_start_array(&writer, refAttrs.size());
    for (auto attr : refAttrs) {
        packString(attr);
    }
    mpack_finish_array(&writer);

    packString("def_attrs");
    mpack_start_array(&writer, defAttrs.size());
    for (auto attr : defAttrs) {
        packString(attr);
    }
    mpack_finish_array(&writer);

    mpack_write_object_bytes(&writer, body, bodySize);
    MPACK_FREE(body);

    mpack_writer_destroy(&writer);

    auto ret = string(header, headerSize);
    MPACK_FREE(header);

    return ret;
}

// Support back-compat down to V2. V3 includes an additional
// symbol for definition Type, namely TypeAlias.
const map<int, int> MsgpackWriter::typeCount{
    {2, 4},
    {3, 5},
    {4, 5},
};

const map<int, vector<string>> MsgpackWriter::refAttrMap{
    {
        2,
        {
            "scope",
            "name",
            "nesting",
            "expression_range",
            "expression_pos_range",
            "resolved",
            "is_defining_ref",
            "parent_of",
        },
    },
    {
        3,
        {
            "scope",
            "name",
            "nesting",
            "expression_range",
            "expression_pos_range",
            "resolved",
            "is_defining_ref",
            "parent_of",
        },
    },
    {
        4,
        {
            "scope",
            "name",
            "nesting",
            "expression_range",
            "expression_pos_range",
            "resolved",
            "is_defining_ref",
            "parent_of",
        },
    },
};

const map<int, vector<string>> MsgpackWriter::defAttrMap{
    {
        2,
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
        3,
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
        4,
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
