#include "main/autogen/subclasses.h"
#include "common/FileOps.h"

using namespace std;
namespace sorbet::autogen {

// Analogue of sorbet::FileOps::isFileIgnored that doesn't take a basePath, since
// we don't need one here, and using FileOps' version meant passing some weird,
// hard-to-understand arguments to mimic how other callers use it.
bool Subclasses::isFileIgnored(const std::string &path, const std::vector<std::string> &absoluteIgnorePatterns,
                               const std::vector<std::string> &relativeIgnorePatterns) {
    for (auto &p : absoluteIgnorePatterns) {
        if (path.substr(0, p.length()) == p && sorbet::FileOps::matchIsFolderOrFile(path, p, 0)) {
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
            } else if (sorbet::FileOps::matchIsFolderOrFile(path, p, pos)) {
                return true;
            }
            pos += p.length();
        }
    }
    return false;
};

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
            fmt::format("{}", fmt::map_join(ref.resolved, "::", [&ctx](const core::NameRef &nm) -> string {
                            return nm.data(ctx)->show(ctx);
                        }));

        // Add child class to the set identified by its parent
        string childName = fmt::format(
            "{}", fmt::map_join(pf.showFullName(ctx, defn),
                                "::", [&ctx](const core::NameRef &nm) -> string { return nm.data(ctx)->show(ctx); }));

        out[parentName].insert(make_pair(childName, defn.data(pf).type));
    }

    return out;
}

// Generate all descendants of a parent class
// Recursively walks `childMap`, which stores the IMMEDIATE children of subclassed class.
optional<Subclasses::Entries> Subclasses::descendantsOf(const Subclasses::Map &childMap, const string &parentName) {
    auto fnd = childMap.find(parentName);
    if (fnd == childMap.end()) {
        return nullopt;
    }
    const Subclasses::Entries children = fnd->second;

    Subclasses::Entries out;
    out.insert(children.begin(), children.end());
    for (const auto &[name, _type] : children) {
        auto descendants = Subclasses::descendantsOf(childMap, name);
        if (descendants) {
            out.insert(descendants->begin(), descendants->end());
        }
    }

    return out;
}

void Subclasses::maybeInsertChild(const string &parentName, const Subclasses::Entries &children, Subclasses::Map &out) {
    if (parentName.empty()) {
        // Child < NonexistentParent
        return;
    }
    out[parentName].insert(children.begin(), children.end());
}

// Manually patch the child map to account for inheritance that happens at runtime `self.included`
// Please do not add to this list.
// TODO(gwu) This shouldn't be public--hide this behind a new public method that realmain.cc calls
void Subclasses::patchChildMap(Subclasses::Map &childMap) {
    childMap["Opus::SafeMachine"].insert(childMap["Opus::Risk::Model::Mixins::RiskSafeMachine"].begin(),
                                         childMap["Opus::Risk::Model::Mixins::RiskSafeMachine"].end());

    childMap["Chalk::SafeMachine"].insert(childMap["Opus::SafeMachine"].begin(), childMap["Opus::SafeMachine"].end());

    childMap["Chalk::ODM::Model"].insert(make_pair("Chalk::ODM::Private::Lock", autogen::Definition::Type::Class));
}

vector<string> Subclasses::serializeSubclassMap(const Subclasses::Map &descendantsMap,
                                                const vector<string> &parentNames) {
    vector<string> descendantsMapSerialized;

    for (const string &parentName : parentNames) {
        auto fnd = descendantsMap.find(parentName);
        if (fnd == descendantsMap.end()) {
            continue;
        }
        const Subclasses::Entries children = fnd->second;

        descendantsMapSerialized.emplace_back(parentName);

        vector<string> serializedChildren;
        for (const auto &[name, type] : children) {
            if (type != autogen::Definition::Type::Class) {
                continue;
            }
            serializedChildren.emplace_back(fmt::format(" {}", name));
        }

        fast_sort(serializedChildren);
        descendantsMapSerialized.insert(descendantsMapSerialized.end(), make_move_iterator(serializedChildren.begin()),
                                        make_move_iterator(serializedChildren.end()));
    }

    return descendantsMapSerialized;
}

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
            descendantsMap[parentName];
        }

        descendantsMap.emplace(parentName, *descendants);
    }

    return Subclasses::serializeSubclassMap(descendantsMap, parentNames);
};

} // namespace sorbet::autogen
