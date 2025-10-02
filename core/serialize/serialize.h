#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::core::serialize {
class Serializer {
public:
    static const uint32_t VERSION = 6;

    static constexpr std::string_view NAME_TABLE_KEY = "NameTable";
    static constexpr std::string_view NAME_TABLE_UUID_KEY = "NameTableUUID";

    // Serialize only the UUID from a global state.
    static std::vector<uint8_t> storeUUID(const GlobalState &gs);

    // Serialize only the name table from a global state. This is suffient for deserializing trees that have only been
    // through the indexing, as they won't have any non-well-known symbols present.
    static std::vector<uint8_t> storeNameTable(const GlobalState &gs);

    struct SerializedGlobalState {
        std::vector<uint8_t> symbolTableData;
        std::vector<uint8_t> nameTableData;
        std::vector<uint8_t> fileTableData;
    };

    // Serialize a global state.
    static SerializedGlobalState store(const GlobalState &gs);

    static std::string fileKey(const core::File &file);

    // Serializes an AST and file hash.
    static std::vector<uint8_t> storeTree(const core::File &file, const ast::ParsedFile &tree);

    // Augment a global state with the name table stored in the cache.
    static void loadAndOverwriteNameTable(GlobalState &gs, const uint8_t *const uuidData, const uint8_t *const data);

    // Load the global state out of buffers that contain the symbol table, name table, and file table. This is only
    // intended to be used for loading the payload out of buffers that are compiled in to the binary.
    static void loadGlobalState(GlobalState &gs, const uint8_t *const symbolTableData,
                                const uint8_t *const nameTableData, const uint8_t *const fileTableData);

    // Initialize only the symbol table of `gs` by deserializing the payload.
    static void loadSymbolTable(GlobalState &gs, const uint8_t *const symbolTableData);

    static uint32_t loadGlobalStateUUID(const GlobalState &gs, const uint8_t *const data);

    // Loads the AST and file hash for the given file. Mutates file to indicate that it is cached and to store the
    // cached file hash.
    // This routine is used for the on-disk key-value cache. The on-disk cache stores these trees at a key that contains
    // the file path along with a hash of the contents of the file. It is possible, but exceedingly unlikely, that the
    // user updates a file to contain new contents but that happens to hash to the same value. If this happens, and the
    // new file is smaller than the old file, Sorbet could encounter memory errors pretty printing `loc`s in error
    // messages as they will point to invalid offsets in the file's text.
    // To prevent that memory error entirely, we stash the file source's text into the `data` blob. If `loadTree`
    // encounters a file with a different sized source text, it returns `nullptr`.
    static ast::ExpressionPtr loadTree(const GlobalState &gs, core::File &file, const uint8_t *const data);
};
}; // namespace sorbet::core::serialize

#endif
