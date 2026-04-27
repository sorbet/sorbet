#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::core::serialize {
class Serializer {
public:
    static const uint32_t VERSION = 6;

    static constexpr std::string_view NAME_TABLE_UUID_KEY = "NameTableUUID";
    static constexpr std::string_view NAME_TABLE_DIFF_COUNT_AND_HASH_SIZE_KEY = "NameTableDiffCountAndHashSize";

    static std::string nameTableDiffKey(uint32_t index);

    // Serialize only the UUID from a global state.
    static std::vector<uint8_t> storeUUID(const GlobalState &gs);

    static std::vector<uint8_t> storeNameTableDiff(const GlobalState &gs);

    static void loadAndAppendNameTableDiff(GlobalState &gs, const uint8_t *const data);

    static std::vector<uint8_t> storeNameTableDiffCountAndHashSize(uint32_t count, const GlobalState &gs);
    static uint32_t loadNameTableDiffCountAndResizeNamesHash(GlobalState &gs, const uint8_t *const data);

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
    // the file path only. We store the hash of the file contents in the value along with the original file size and the
    // serialized tree. It is possible, but exceedingly unlikely, that the user updates a file to contain new contents
    // but that happens to hash to the same value. If this happens, and the new file is smaller than the old file,
    // Sorbet could encounter memory errors pretty printing `loc`s in error messages as they will point to invalid
    // offsets in the file's text. To prevent that memory error entirely, we compare the file hash and its source
    // length. If `loadTree` encounters a file with a different sized source text, it returns `nullptr`.
    static ast::ExpressionPtr loadTree(const GlobalState &gs, core::File &file, const uint8_t *const data);
};
}; // namespace sorbet::core::serialize

#endif
