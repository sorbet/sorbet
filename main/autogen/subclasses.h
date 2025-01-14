#ifndef AUTOGEN_SUBCLASSES_H
#define AUTOGEN_SUBCLASSES_H
#include "common/common.h"
#include "main/autogen/data/definitions.h"

namespace sorbet::autogen {

class Subclasses final {
public:
    // A set of child classes for a particular parent.
    struct ChildInfo {
        std::optional<core::Loc> defining_ref;

        void mergeDefiningRefWith(core::Loc loc);
    };

    using Entries = UnorderedMap<core::ClassOrModuleRef, ChildInfo>;

    struct SubclassInfo {
        // This is weird, because this is information about the *parent*, not the child.
        ClassKind classKind = ClassKind::Module;
        Entries entries;

        SubclassInfo() = default;
        SubclassInfo(ClassKind classKind, Entries entries) : classKind(classKind), entries(std::move(entries)){};
    };

    // Map between the subclass seen to information about the parent class
    using Map = UnorderedMap<core::SymbolRef, SubclassInfo>;

    static void mergeInto(Entries &out, const Entries &entries);
    static std::optional<Subclasses::Map> listAllSubclasses(core::Context ctx, const ParsedFile &pf,
                                                            const std::vector<std::string> &absoluteIgnorePatterns,
                                                            const std::vector<std::string> &relativeIgnorePattern);
    static std::vector<std::string> genDescendantsMap(const core::GlobalState &gs, Subclasses::Map &childMap,
                                                      std::vector<core::SymbolRef> &parentRefs);
    static const core::SymbolRef getConstantRef(const core::GlobalState &gs, std::string rawName);

private:
    static void patchChildMap(const core::GlobalState &gs, Subclasses::Map &childMap);
    static bool isFileIgnored(const std::string &path, const std::vector<std::string> &absoluteIgnorePatterns,
                              const std::vector<std::string> &relativeIgnorePatterns);
    static std::optional<SubclassInfo> descendantsOf(const Subclasses::Map &childMap, const core::SymbolRef &parentRef);
    static std::vector<std::string> serializeSubclassMap(const core::GlobalState &gs,
                                                         const Subclasses::Map &descendantsMap,
                                                         const std::vector<core::SymbolRef> &parentNames);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_SUBCLASSES_H
