#ifndef AUTOGEN_SUBCLASSES_H
#define AUTOGEN_SUBCLASSES_H
#include "common/common.h"
#include "main/autogen/autogen.h"

namespace sorbet::autogen {

class Subclasses final {
public:
    typedef std::pair<std::string, Definition::Type> Entry;
    typedef UnorderedSet<Entry> Entries;
    typedef UnorderedMap<std::string, Entries> Map;

    static std::optional<Subclasses::Map> listAllSubclasses(core::Context ctx, ParsedFile &pf,
                                                            const std::vector<std::string> &absolutePathsToIgnore,
                                                            const std::vector<std::string> &relativePathsToIgnore);

    static void maybeInsertChild(const std::string &parentName, const Subclasses::Entries &children,
                                 Subclasses::Map &out);
    static void descendantsOf(const Subclasses::Map &childMap, const std::string &parent, Subclasses::Entries &out);
    static std::vector<std::string> serializeSubclassMap(const Subclasses::Map &descendantsMap,
                                                         const std::vector<std::string> &parentNames);
    static std::vector<std::string> genDescendantsMap(Subclasses::Map &childMap, std::vector<std::string> &parentNames);

private:
    static void patchChildMap(Subclasses::Map &childMap);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_SUBCLASSES_H
