#include "main/autogen/data/definitions.h"
#include "ast/ast.h"
#include "common/formatting.h"
#include "core/Files.h"
#include "core/GlobalState.h"
#include "main/autogen/data/msgpack.h"

using namespace std;

namespace sorbet::autogen {

QualifiedName QualifiedName::fromFullName(vector<core::NameRef> &&name) {
    return QualifiedName{move(name)};
}

string QualifiedName::show(const core::GlobalState &gs) const {
    return fmt::format(
        "::{}", fmt::map_join(nameParts, "::", [&](core::NameRef nr) -> string_view { return nr.shortName(gs); }));
}

string QualifiedName::join(const core::GlobalState &gs, string_view sep) const {
    return fmt::format("{}", fmt::map_join(nameParts, sep, [&](core::NameRef nr) -> string { return nr.show(gs); }));
}

// Find the `Definition` associated with this `DefinitionRef`
const Definition &DefinitionRef::data(const ParsedFile &pf) const {
    return pf.defs[_id];
}

// Find the `Reference` associated with this `ReferenceRef`
const Reference &ReferenceRef::data(const ParsedFile &pf) const {
    return pf.refs[_id];
}

// Show the full qualified name of this `Definition`, if possible. (If for some reason there is no actual `defining_ref`
// for this `DefinitionRef` (e.g. if this is called _during_ an `AutogenWalk` traversal and the defining ref has not yet
// been set) then this will return an empty vector.
vector<core::NameRef> ParsedFile::showFullName(const core::GlobalState &gs, DefinitionRef id) const {
    auto &def = id.data(*this);
    if (!def.defining_ref.exists()) {
        return {};
    }
    auto &ref = def.defining_ref.data(*this);
    auto scope = showFullName(gs, ref.scope);
    scope.insert(scope.end(), ref.name.nameParts.begin(), ref.name.nameParts.end());
    return scope;
}

// Show the full qualified name of this `Definition`, if possible. (If for some reason there is no actual `defining_ref`
// for this `DefinitionRef` (e.g. if this is called _during_ an `AutogenWalk` traversal and the defining ref has not yet
// been set) then this will return an empty vector.
QualifiedName ParsedFile::showQualifiedName(const core::GlobalState &gs, DefinitionRef id) const {
    auto &def = id.data(*this);
    if (!def.defining_ref.exists()) {
        return {};
    }
    auto &ref = def.defining_ref.data(*this);
    auto nameParts = showFullName(gs, ref.scope);
    nameParts.insert(nameParts.end(), ref.name.nameParts.begin(), ref.name.nameParts.end());
    return QualifiedName::fromFullName(move(nameParts));
}

// Pretty-print a `ParsedFile`, including all definitions and references and the pieces of metadata associated with them
string ParsedFile::toString(const core::GlobalState &gs, int version) const {
    fmt::memory_buffer out;
    auto nameToString = [&](const auto &nm) -> string_view { return nm.shortName(gs); };

    fmt::format_to(std::back_inserter(out),
                   "# ParsedFile: {}\n"
                   "requires: [{}]\n"
                   "## defs:\n",
                   core::File::censorFilePathForSnapshotTests(path),
                   fmt::map_join(requireStatements, ", ", nameToString));

    for (auto &def : defs) {
        string_view type;
        switch (def.type) {
            case Definition::Type::Module:
                type = "module"sv;
                break;
            case Definition::Type::Class:
                type = "class"sv;
                break;
            case Definition::Type::Casgn:
                type = "casgn"sv;
                break;
            case Definition::Type::Alias:
                type = "alias"sv;
                break;
            case Definition::Type::TypeAlias:
                if (version <= 2) {
                    type = "casgn"sv;
                } else {
                    type = "typealias"sv;
                }
                break;
        }

        fmt::format_to(std::back_inserter(out),
                       "[def id={}]\n"
                       " type={}\n"
                       " defines_behavior={}\n"
                       " is_empty={}\n",
                       def.id.id(), type, (int)def.defines_behavior, (int)def.is_empty);

        if (def.defining_ref.exists()) {
            auto &ref = def.defining_ref.data(*this);
            fmt::format_to(std::back_inserter(out), " defining_ref=[{}]\n",
                           fmt::map_join(ref.name.nameParts, " ", nameToString));
        }
        if (def.parent_ref.exists()) {
            auto &ref = def.parent_ref.data(*this);
            fmt::format_to(std::back_inserter(out), " parent_ref=[{}]\n",
                           fmt::map_join(ref.name.nameParts, " ", nameToString));
        }
        if (def.aliased_ref.exists()) {
            auto &ref = def.aliased_ref.data(*this);
            fmt::format_to(std::back_inserter(out), " aliased_ref=[{}]\n",
                           fmt::map_join(ref.name.nameParts, " ", nameToString));
        }
    }
    fmt::format_to(std::back_inserter(out), "## refs:\n");
    for (auto &ref : refs) {
        vector<string> nestingStrings;
        for (auto &scope : ref.nesting) {
            auto fullScopeName = showFullName(gs, scope);
            nestingStrings.emplace_back(fmt::format("[{}]", fmt::map_join(fullScopeName, " ", nameToString)));
        }

        auto refFullName = showFullName(gs, ref.scope);
        fmt::format_to(std::back_inserter(out),
                       "[ref id={}]\n"
                       " scope=[{}]\n"
                       " name=[{}]\n"
                       " nesting=[{}]\n"
                       " resolved=[{}]\n"
                       " loc={}\n"
                       " is_defining_ref={}\n",

                       ref.id.id(), fmt::map_join(refFullName, " ", nameToString),
                       fmt::map_join(ref.name.nameParts, " ", nameToString), fmt::join(nestingStrings, " "),
                       fmt::map_join(ref.resolved.nameParts, " ", nameToString),
                       core::Loc(tree.file, ref.loc).filePosToString(gs), (int)ref.is_defining_ref);

        if (ref.parent_of.exists()) {
            auto parentOfFullName = showFullName(gs, ref.parent_of);
            fmt::format_to(std::back_inserter(out), " parent_of=[{}]\n",
                           fmt::map_join(parentOfFullName, " ", nameToString));
        }
    }
    return to_string(out);
}

// List every class name defined in this `ParsedFile`.
vector<string> ParsedFile::listAllClasses(core::Context ctx) {
    vector<string> out;

    for (auto &def : defs) {
        if (def.type != Definition::Type::Class) {
            continue;
        }
        vector<core::NameRef> names = showFullName(ctx, def.id);
        out.emplace_back(fmt::format("{}", fmt::map_join(names, "::", [&ctx](const core::NameRef &nm) -> string_view {
                                         return nm.shortName(ctx);
                                     })));
    }

    return out;
}

// Convert this parsedfile to a msgpack representation
string ParsedFile::toMsgpack(core::Context ctx, int version, const AutogenConfig &autogenCfg) {
    MsgpackWriter write(version);
    return write.pack(ctx, *this, autogenCfg);
}

} // namespace sorbet::autogen
