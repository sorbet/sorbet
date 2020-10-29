#ifndef AUTOGEN_SUBCLASSES_H
#define AUTOGEN_SUBCLASSES_H
#include "common/common.h"
#include "main/autogen/data/definitions.h"

namespace sorbet::autogen {

class Subclasses final {
public:
    using Entry = std::pair<std::string, Definition::Type>;
    using Entries = UnorderedSet<Entry>;
    struct SubclassInfo {
        ClassKind classKind = ClassKind::Module;
        Entries entries;

        SubclassInfo() = default;
        SubclassInfo(ClassKind classKind, Entries entries) : classKind(classKind), entries(entries){};
    };
    using Map = UnorderedMap<std::string, SubclassInfo>;

    static std::optional<Subclasses::Map> listAllSubclasses(core::Context ctx, ParsedFile &pf,
                                                            const std::vector<std::string> &absoluteIgnorePatterns,
                                                            const std::vector<std::string> &relativeIgnorePatterns);
    static std::vector<std::string> genDescendantsMap(Subclasses::Map &childMap, std::vector<std::string> &parentNames);

private:
    static void patchChildMap(Subclasses::Map &childMap);
    static bool isFileIgnored(const std::string &path, const std::vector<std::string> &absoluteIgnorePatterns,
                              const std::vector<std::string> &relativeIgnorePatterns);
    static std::optional<SubclassInfo> descendantsOf(const Subclasses::Map &childMap, const std::string &parents);
    static std::vector<std::string> serializeSubclassMap(const Subclasses::Map &descendantsMap,
                                                         const std::vector<std::string> &parentNames);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_SUBCLASSES_H
