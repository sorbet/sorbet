#ifndef AUTOGEN_MSGPACK_H
#define AUTOGEN_MSGPACK_H
// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "main/autogen/data/definitions.h"
#include "main/autogen/data/version.h"

namespace sorbet::autogen {

class MsgpackWriter {
private:
    int version;
    const std::vector<std::string> &refAttrs;
    const std::vector<std::string> &defAttrs;
    mpack_writer_t writer;

    std::vector<core::NameRef> symbols;
    UnorderedMap<core::NameRef, uint32_t> symbolIds;

    static const std::map<int, std::vector<std::string>> refAttrMap;
    static const std::map<int, std::vector<std::string>> defAttrMap;
    static const std::map<int, int> typeCount;

    // a bunch of helpers
    void packName(core::NameRef nm);
    void packNames(std::vector<core::NameRef> &names);
    void packString(std::string_view str);
    void packBool(bool b);
    void packReferenceRef(ReferenceRef ref);
    void packDefinitionRef(DefinitionRef ref);
    void packRange(uint32_t begin, uint32_t end);
    void packDefinition(core::Context ctx, ParsedFile &pf, Definition &def, const AutogenConfig &autogenCfg);
    void packReference(core::Context ctx, ParsedFile &pf, Reference &ref);
    static int assertValidVersion(int version) {
        if (version < AutogenVersion::MIN_VERSION || version > AutogenVersion::MAX_VERSION) {
            Exception::raise("msgpack version {} not in available range [{}, {}]", version, AutogenVersion::MIN_VERSION,
                             AutogenVersion::MAX_VERSION);
        }
        return version;
    }

public:
    MsgpackWriter(int version);

    std::string pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg);
};

} // namespace sorbet::autogen

#endif // AUTOGEN_MSGPACK_H
