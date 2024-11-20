#ifndef AUTOGEN_MSGPACK_H
#define AUTOGEN_MSGPACK_H
// has to go first because it violates our poisons
#include "mpack/mpack.h"

#include "main/autogen/data/definitions.h"
#include "main/autogen/data/version.h"

namespace sorbet::autogen {

class MsgpackWriter {
protected:
    int version;
    const std::vector<std::string> &refAttrs;
    const std::vector<std::string> &defAttrs;
    const std::vector<std::string> &pfAttrs;

    std::vector<core::NameRef> symbols;
    UnorderedMap<core::NameRef, uint32_t> symbolIds;

    static const std::map<int, std::vector<std::string>> refAttrMap;
    static const std::map<int, std::vector<std::string>> defAttrMap;
    static const std::map<int, std::vector<std::string>> parsedFileAttrMap;

    // a bunch of helpers
    void packName(mpack_writer_t *writer, core::NameRef nm);
    void packNames(mpack_writer_t *writer, std::vector<core::NameRef> &names);
    void packBool(mpack_writer_t *writer, bool b);
    void packReferenceRef(mpack_writer_t *writer, ReferenceRef ref);
    void packDefinitionRef(mpack_writer_t *writer, DefinitionRef ref);
    void packRange(mpack_writer_t *writer, uint32_t begin, uint32_t end);
    virtual void packDefinition(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Definition &def,
                                const AutogenConfig &autogenCfg);
    virtual void packReference(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Reference &ref);
    static int assertValidVersion(int version) {
        if (version < AutogenVersion::MIN_VERSION || version > AutogenVersion::MAX_VERSION) {
            Exception::raise("msgpack version {} not in available range [{}, {}]", version, AutogenVersion::MIN_VERSION,
                             AutogenVersion::MAX_VERSION);
        }
        return version;
    }

public:
    MsgpackWriter(int version);

    static std::string msgpackGlobalHeader(int version, size_t numFiles);
    virtual std::string pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg);
};

// Lightweight version of writer that skips all reference metadata like expression ranges, inheritance information,
// typing information, etc. Reduces size by ~37% for Stripe code.
class MsgpackWriterLite : public MsgpackWriter {
private:
    int version;
    const std::vector<std::string> &refAttrs;
    const std::vector<std::string> &defAttrs;
    const std::vector<std::string> &pfAttrs;

    static const std::map<int, std::vector<std::string>> refAttrMap;
    static const std::map<int, std::vector<std::string>> defAttrMap;
    static const std::map<int, std::vector<std::string>> parsedFileAttrMap;

    static int assertValidVersion(int version) {
        // Lite Msgpack writer is only supported after version 6.
        if (version < 6 && version > AutogenVersion::MAX_VERSION) {
            Exception::raise("msgpack version {} not in available range [6, {}]", version, 6,
                             AutogenVersion::MAX_VERSION);
        }
        return version;
    }

    void packDefinition(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Definition &def,
                        const AutogenConfig &autogenCfg);
    void packReference(mpack_writer_t *writer, core::Context ctx, ParsedFile &pf, Reference &ref);

public:
    MsgpackWriterLite(int version);

    static std::string msgpackGlobalHeader(int version, size_t numFiles);
    std::string pack(core::Context ctx, ParsedFile &pf, const AutogenConfig &autogenCfg);
};

void packString(mpack_writer_t *writer, std::string_view str);
void writeSymbols(core::Context ctx, mpack_writer_t *writer, const std::vector<core::NameRef> &symbols);
std::string buildGlobalHeader(int version, int serializedVersion, size_t numFiles,
                              const std::vector<std::string> &refAttrs, const std::vector<std::string> &defAttrs,
                              const std::vector<std::string> &pfAttrs);

} // namespace sorbet::autogen

#endif // AUTOGEN_MSGPACK_H
