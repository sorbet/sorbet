// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "main/autogen/data/definitions.h"
#include "main/autogen/data/msgpack.h"

using namespace std;
namespace sorbet::autogen {

void MsgpackWriter::packName(core::NameRef nm) {
    u4 id;
    auto it = symbolIds.find(nm);
    if (it == symbolIds.end()) {
        id = symbols.size();
        symbols.emplace_back(nm);
        symbolIds[nm] = id;
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

void MsgpackWriter::packRange(u4 begin, u4 end) {
    mpack_write_u64(&writer, ((u8)begin << 32) | end);
}

void MsgpackWriter::packDefinition(core::Context ctx, ParsedFile &pf, Definition &def) {
    mpack_start_array(&writer, defAttrs[version].size());

    // raw_full_name
    auto raw_full_name = pf.showFullName(ctx, def.id);
    packNames(raw_full_name);

    // type
    mpack_write_u8(&writer, static_cast<u8>(def.type));

    // defines_behavior
    packBool(def.defines_behavior);

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
    auto expression_range = core::Loc(ctx.file, ref.definitionLoc).position(ctx);
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

// symbols[0..3] are reserved for the Type aliases
MsgpackWriter::MsgpackWriter(int version)
    : version(assertValidVersion(version)), refAttrs(refAttrMap.at(version)), defAttrs(defAttrMap.at(version)),
      symbols(4) {}

string MsgpackWriter::pack(core::Context ctx, ParsedFile &pf) {
    char *data;
    size_t size;
    mpack_writer_init_growable(&writer, &data, &size);
    mpack_start_array(&writer, 6);

    mpack_write_true(&writer); // did_resolution
    packString(pf.path);
    mpack_write_u32(&writer, pf.cksum);

    // requires
    mpack_start_array(&writer, pf.requires.size());
    for (auto nm : pf.requires) {
        packString(nm.show(ctx));
    }
    mpack_finish_array(&writer);

    mpack_start_array(&writer, pf.defs.size());
    for (auto &def : pf.defs) {
        packDefinition(ctx, pf, def);
    }

    mpack_finish_array(&writer);
    mpack_start_array(&writer, pf.refs.size());
    for (auto &ref : pf.refs) {
        packReference(ctx, pf, ref);
    }
    mpack_finish_array(&writer);
    mpack_finish_array(&writer);

    mpack_writer_destroy(&writer);
    auto body = string(data, size);
    MPACK_FREE(data);

    // write header
    mpack_writer_init_growable(&writer, &data, &size);

    mpack_start_map(&writer, 5);

    packString("symbols");
    int i = -1;
    mpack_start_array(&writer, symbols.size());
    for (auto sym : symbols) {
        ++i;
        string str;
        switch ((Definition::Type)i) {
            case Definition::Type::Module:
                str = "module";
                break;
            case Definition::Type::Class:
                str = "class";
                break;
            case Definition::Type::Casgn:
                str = "casgn";
                break;
            case Definition::Type::Alias:
                str = "alias";
                break;
            default:
                str = sym.shortName(ctx);
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

    mpack_write_object_bytes(&writer, body.data(), body.size());

    mpack_writer_destroy(&writer);

    auto ret = string(data, size);
    MPACK_FREE(data);

    return ret;
}

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
};

} // namespace sorbet::autogen
