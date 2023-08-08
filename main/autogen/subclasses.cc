#include "main/autogen/subclasses.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/GlobalState.h"
#include "core/Unfreeze.h"

using namespace std;
namespace sorbet::autogen {

// Analogue of sorbet::FileOps::isFileIgnored that doesn't take a basePath, since
// we don't need one here, and using FileOps' version meant passing some weird,
// hard-to-understand arguments to mimic how other callers use it.
bool Subclasses::isFileIgnored(const std::string &path, const std::vector<std::string> &absoluteIgnorePatterns,
                               const std::vector<std::string> &relativeIgnorePatterns) {
    for (auto &p : absoluteIgnorePatterns) {
        if (path.substr(0, p.length()) == p &&
            (sorbet::FileOps::isFile(path, p, 0) || sorbet::FileOps::isFolder(path, p, 0))) {
            return true;
        }
    }
    for (auto &p : relativeIgnorePatterns) {
        // See if /pattern is in string, and that it matches a whole folder or file name.
        int pos = 0;
        while (true) {
            pos = path.find(p, pos);
            if (pos == string_view::npos) {
                break;
            } else if (sorbet::FileOps::isFile(path, p, pos) || sorbet::FileOps::isFolder(path, p, pos)) {
                return true;
            }
            pos += p.length();
        }
    }
    return false;
};

// Convert a symbol name into a fully qualified name
vector<core::NameRef> Subclasses::symbolName(core::GlobalState &gs, core::SymbolRef sym) {
    vector<core::NameRef> out;
    while (sym.exists() && sym != core::Symbols::root()) {
        out.emplace_back(sym.name(gs));
        sym = sym.owner(gs);
    }
    reverse(out.begin(), out.end());
    return out;
}

// Get all subclasses defined in a particular ParsedFile
optional<Subclasses::Map> Subclasses::listAllSubclasses(core::Context ctx, const ParsedFile &pf,
                                                        const vector<string> &absoluteIgnorePatterns,
                                                        const vector<string> &relativeIgnorePatterns) {
    // Prepend "/" because absoluteIgnorePatterns and relativeIgnorePatterns are always "/"-prefixed
    if (isFileIgnored(fmt::format("/{}", pf.path), absoluteIgnorePatterns, relativeIgnorePatterns)) {
        return nullopt;
    }

    Subclasses::Map out;
    for (const Reference &ref : pf.refs) {
        DefinitionRef defn = ref.parent_of;
        if (!defn.exists()) {
            // This is just a random constant reference and doesn't
            // define a Child < Parent relationship.
            continue;
        }

        auto &mapEntry = out[ref.sym];
        mapEntry.entries.insert({defn.data(pf), pf.file});
        mapEntry.classKind = ref.parentKind;
    }

    return out;
}

// Generate all descendants of a parent class
// Recursively walks `childMap`, which stores the IMMEDIATE children of subclassed class.
optional<Subclasses::SubclassInfo> Subclasses::descendantsOf(const Subclasses::Map &childMap,
                                                             const core::SymbolRef &parentRef) {
    auto fnd = childMap.find(parentRef);
    if (fnd == childMap.end()) {
        return nullopt;
    }
    const Subclasses::Entries &children = fnd->second.entries;

    Subclasses::Entries out;
    out.insert(children.begin(), children.end());
    for (const auto &[def, file] : children) {
        auto descendants = Subclasses::descendantsOf(childMap, def.sym);
        if (descendants) {
            out.insert(descendants->entries.begin(), descendants->entries.end());
        }
    }

    return SubclassInfo(fnd->second.classKind, std::move(out));
}

const core::SymbolRef Subclasses::getConstantRef(core::GlobalState &gs, string rawName) {
    core::UnfreezeNameTable nameTableAccess(gs);
    core::UnfreezeSymbolTable symbolTableAccess(gs);
    core::ClassOrModuleRef sym = core::Symbols::root();

    for (auto &n : absl::StrSplit(rawName, "::")) {
        sym = gs.enterClassSymbol(core::Loc(), sym, gs.enterNameConstant(gs.enterNameUTF8(n)));
    }
    return sym;
}

// Manually patch the child map to account for inheritance that happens at runtime `self.included`
// Please do not add to this list.
void Subclasses::patchChildMap(core::GlobalState &gs, Subclasses::Map &childMap) {
    auto riskSafeMachineRef = getConstantRef(gs, "Opus::Risk::Model::Mixins::RiskSafeMachine");
    childMap[getConstantRef(gs, "Opus::SafeMachine")].entries.insert(childMap[riskSafeMachineRef].entries.begin(),
                                                                     childMap[riskSafeMachineRef].entries.end());
}

vector<string> Subclasses::serializeSubclassMap(core::GlobalState &gs, const Subclasses::Map &descendantsMap,
                                                const vector<core::SymbolRef> &parentNames, const bool showPaths) {
    vector<string> descendantsMapSerialized;
    const auto classFormatString = showPaths ? " class {} {}" : " class {}";
    const auto moduleFormatString = showPaths ? " module {} {}" : " module {}";

    for (const auto &parentRef : parentNames) {
        auto fnd = descendantsMap.find(parentRef);
        if (fnd == descendantsMap.end()) {
            continue;
        }
        const Subclasses::SubclassInfo &children = fnd->second;

        string parentName =
            fmt::format("{}", fmt::map_join(symbolName(gs, parentRef),
                                            "::", [&gs](const core::NameRef &nm) -> string { return nm.show(gs); }));

        auto type = children.classKind == ClassKind::Class ? "class" : "module";
        descendantsMapSerialized.emplace_back(fmt::format("{} {}", type, parentName));

        auto subclassesStart = descendantsMapSerialized.size();
        for (const auto &[def, file] : children.entries) {
            string_view path = file.data(gs).path();
            // Strip initial "./" from path
            if (path.find("./") == 0)
                path = path.substr(2);

            string childName = fmt::format(
                "{}", fmt::map_join(symbolName(gs, def.sym),
                                    "::", [&gs](const core::NameRef &nm) -> string { return nm.show(gs); }));

            // Ignore Modules
            if (def.type == autogen::Definition::Type::Class) {
                // Note: fmt ignores excess arguments
                descendantsMapSerialized.emplace_back(fmt::format(classFormatString, childName, path));
            } else {
                descendantsMapSerialized.emplace_back(fmt::format(moduleFormatString, childName, path));
            }
        }

        fast_sort_range(descendantsMapSerialized.begin() + subclassesStart, descendantsMapSerialized.end());
    }

    return descendantsMapSerialized;
}

// Generate a list of strings representing the descendants of a given list of parent classes
//
// e.g.
// Parent1
//  Child1
// Parent2
//  Child2
//  Child3
//
// This effectively replaces pay-server's `DescendantsMap` in `InheritedClassesStep` with a much
// faster implementation.
vector<string> Subclasses::genDescendantsMap(core::GlobalState &gs, Subclasses::Map &childMap,
                                             vector<core::SymbolRef> &parentRefs, const bool showPaths) {
    Subclasses::patchChildMap(gs, childMap);

    // Generate descendants for each passed-in superclass
    fast_sort(parentRefs, [&gs](const auto &left, const auto &right) { return left.show(gs) < right.show(gs); });
    Subclasses::Map descendantsMap;
    for (const auto &parentRef : parentRefs) {
        // Skip parents that the user asked for but which don't
        // exist or are never subclassed.
        auto fnd = childMap.find(parentRef);
        if (fnd == childMap.end()) {
            continue;
        }

        auto descendants = Subclasses::descendantsOf(childMap, parentRef);
        if (!descendants) {
            // Initialize an empty entry associated with parentName.
            descendantsMap[parentRef];
        } else {
            descendantsMap.emplace(parentRef, std::move(*descendants));
        }
    }

    return Subclasses::serializeSubclassMap(gs, descendantsMap, parentRefs, showPaths);
};

} // namespace sorbet::autogen
