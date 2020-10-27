#ifndef AUTOGEN_MSGPACK_H
#define AUTOGEN_MSGPACK_H
// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "main/autogen/autogen.h"

namespace sorbet::autogen {

class MsgpackWriter {
private:
    int version;
    const std::vector<std::string> &refAttrs;
    const std::vector<std::string> &defAttrs;
    mpack_writer_t writer;

    std::vector<core::NameRef> symbols;
    UnorderedMap<core::NameRef, u4> symbolIds;

    static const std::map<int, std::vector<std::string>> refAttrMap;
    static const std::map<int, std::vector<std::string>> defAttrMap;

    // a bunch of helpers
    void packName(core::NameRef nm);
    void packNames(std::vector<core::NameRef> &names);
    void packString(std::string_view str);
    void packBool(bool b);
    void packReferenceRef(ReferenceRef ref);
    void packDefinitionRef(DefinitionRef ref);
    void packRange(u4 begin, u4 end);
    void packDefinition(core::Context ctx, ParsedFile &pf, Definition &def);
    void packReference(core::Context ctx, ParsedFile &pf, Reference &ref);
    static int assertValidVersion(int version) {
        if (version < MIN_VERSION || version > MAX_VERSION) {
            Exception::raise("msgpack version {} not in available range [{}, {}]", version, MIN_VERSION, MAX_VERSION);
        }
        return version;
    }

public:
    MsgpackWriter(int version);

    constexpr static int MIN_VERSION = 2;
    constexpr static int MAX_VERSION = 2;
    std::string pack(core::Context ctx, ParsedFile &pf);
};

} // namespace sorbet::autogen

#endif // AUTOGEN_MSGPACK_H
