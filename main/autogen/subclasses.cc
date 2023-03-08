#include "main/autogen/subclasses.h"
#include "common/FileOps.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "core/GlobalState.h"

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

// Get all subclasses defined in a particular ParsedFile
optional<Subclasses::Map> Subclasses::listAllSubclasses(core::Context ctx, ParsedFile &pf,
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

        // Get fully-qualified parent name as string
        string parentName =
            fmt::format("{}", fmt::map_join(ref.resolved.nameParts,
                                            "::", [&ctx](const core::NameRef &nm) -> string { return nm.show(ctx); }));

        // Add child class to the set identified by its parent
        string childName =
            fmt::format("{}", fmt::map_join(pf.showFullName(ctx, defn),
                                            "::", [&ctx](const core::NameRef &nm) -> string { return nm.show(ctx); }));

        auto &mapEntry = out[parentName];
        mapEntry.entries.insert(make_pair(childName, defn.data(pf).type));
        mapEntry.classKind = ref.parentKind;
    }

    return out;
}

// Generate all descendants of a parent class
// Recursively walks `childMap`, which stores the IMMEDIATE children of subclassed class.
optional<Subclasses::SubclassInfo> Subclasses::descendantsOf(const Subclasses::Map &childMap,
                                                             const string &parentName) {
    auto fnd = childMap.find(parentName);
    if (fnd == childMap.end()) {
        return nullopt;
    }
    const Subclasses::Entries &children = fnd->second.entries;

    Subclasses::Entries out;
    out.insert(children.begin(), children.end());
    for (const auto &[name, _type] : children) {
        auto descendants = Subclasses::descendantsOf(childMap, name);
        if (descendants) {
            out.insert(descendants->entries.begin(), descendants->entries.end());
        }
    }

    return SubclassInfo(fnd->second.classKind, std::move(out));
}

// Manually patch the child map to account for inheritance that happens at runtime `self.included`
// Please do not add to this list.
void Subclasses::patchChildMap(Subclasses::Map &childMap) {
    childMap["Opus::SafeMachine"].entries.insert(childMap["Opus::Risk::Model::Mixins::RiskSafeMachine"].entries.begin(),
                                                 childMap["Opus::Risk::Model::Mixins::RiskSafeMachine"].entries.end());
}

vector<string> Subclasses::serializeSubclassMap(const Subclasses::Map &descendantsMap,
                                                const vector<string> &parentNames) {
    vector<string> descendantsMapSerialized;

    for (const string &parentName : parentNames) {
        auto fnd = descendantsMap.find(parentName);
        if (fnd == descendantsMap.end()) {
            continue;
        }
        const Subclasses::SubclassInfo &children = fnd->second;

        auto type = children.classKind == ClassKind::Class ? "class" : "module";
        descendantsMapSerialized.emplace_back(fmt::format("{} {}", type, parentName));

        auto subclassesStart = descendantsMapSerialized.size();
        for (const auto &[name, type] : children.entries) {
            // Ignore Modules
            if (type == autogen::Definition::Type::Class) {
                descendantsMapSerialized.emplace_back(fmt::format(" class {}", name));
            } else {
                descendantsMapSerialized.emplace_back(fmt::format(" module {}", name));
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
vector<string> Subclasses::genDescendantsMap(Subclasses::Map &childMap, vector<string> &parentNames) {
    Subclasses::patchChildMap(childMap);

    // Generate descendants for each passed-in superclass
    fast_sort(parentNames);
    Subclasses::Map descendantsMap;
    for (const string &parentName : parentNames) {
        // Skip parents that the user asked for but which don't
        // exist or are never subclassed.
        auto fnd = childMap.find(parentName);
        if (fnd == childMap.end()) {
            continue;
        }

        auto descendants = Subclasses::descendantsOf(childMap, parentName);
        if (!descendants) {
            // Initialize an empty entry associated with parentName.
            descendantsMap[parentName];
        } else {
            descendantsMap.emplace(parentName, std::move(*descendants));
        }
    }

    return Subclasses::serializeSubclassMap(descendantsMap, parentNames);
};

} // namespace sorbet::autogen
