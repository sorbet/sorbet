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
        mapEntry.entries.insert(defn.data(pf).sym.asClassOrModuleRef());
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
    for (const auto &childRef : children) {
        auto descendants = Subclasses::descendantsOf(childMap, childRef);
        if (descendants) {
            out.insert(descendants->entries.begin(), descendants->entries.end());
        }
    }

    return SubclassInfo(fnd->second.classKind, std::move(out));
}

const core::SymbolRef Subclasses::getConstantRef(const core::GlobalState &gs, string rawName) {
    core::ClassOrModuleRef sym = core::Symbols::root();

    for (auto &n : absl::StrSplit(rawName, "::")) {
        const auto &nameRef = gs.lookupNameUTF8(n);
        if (!nameRef.exists())
            return core::Symbols::noSymbol();
        sym = gs.lookupClassSymbol(sym, gs.lookupNameConstant(nameRef));
    }
    return sym;
}

// Manually patch the child map to account for inheritance that happens at runtime `self.included`
// Please do not add to this list.
void Subclasses::patchChildMap(const core::GlobalState &gs, Subclasses::Map &childMap) {
    auto safeMachineRef = getConstantRef(gs, "Opus::SafeMachine");
    auto riskSafeMachineRef = getConstantRef(gs, "Opus::Risk::Model::Mixins::RiskSafeMachine");
    if (!safeMachineRef.exists() || !riskSafeMachineRef.exists())
        return;
    childMap[safeMachineRef].entries.insert(childMap[riskSafeMachineRef].entries.begin(),
                                            childMap[riskSafeMachineRef].entries.end());
}

vector<string> Subclasses::serializeSubclassMap(const core::GlobalState &gs, const Subclasses::Map &descendantsMap,
                                                const vector<core::SymbolRef> &parentNames) {
    vector<string> descendantsMapSerialized;
    for (const auto &parentRef : parentNames) {
        auto fnd = descendantsMap.find(parentRef);
        if (fnd == descendantsMap.end()) {
            continue;
        }
        const Subclasses::SubclassInfo &children = fnd->second;

        string parentName = parentRef.show(gs);

        auto type = children.classKind == ClassKind::Class ? "class" : "module";
        descendantsMapSerialized.emplace_back(fmt::format("{} {}", type, parentName));

        auto subclassesStart = descendantsMapSerialized.size();
        for (const auto &childRef : children.entries) {
            string_view path = gs.getPrintablePath(childRef.data(gs)->loc().file().data(gs).path());
            string childName = childRef.show(gs);
            auto type = childRef.data(gs)->isClass() ? "class" : "module";
            descendantsMapSerialized.emplace_back(fmt::format(" {} {} {}", type, childName, path));
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
vector<string> Subclasses::genDescendantsMap(const core::GlobalState &gs, Subclasses::Map &childMap,
                                             vector<core::SymbolRef> &parentRefs) {
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

    return Subclasses::serializeSubclassMap(gs, descendantsMap, parentRefs);
};

} // namespace sorbet::autogen
