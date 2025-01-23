#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::core::serialize {
class Serializer {
public:
    static const uint32_t VERSION = 6;

    static constexpr std::string_view GLOBAL_STATE_KEY = "GlobalState";

    // Serialize only the name table from a global state. This is suffient for deserializing trees that have only been
    // through the indexing, as they won't have any non-well-known symbols present.
    static std::vector<uint8_t> storeNameTable(const GlobalState &gs);

    // Serialize a global state.
    static std::vector<uint8_t> store(const GlobalState &gs);

    // Stores a GlobalState, but only includes `File`s with Type == Payload.
    // This can be used in conjunction with `storeFile` to store
    // a global state containing a name table along side a large number of
    // individual cached files, which can be loaded independently.
    static std::vector<uint8_t> storePayloadAndNameTable(const GlobalState &gs);

    static std::string fileKey(const core::File &file);

    // Serializes an AST and file hash.
    static std::vector<uint8_t> storeTree(const core::File &file, const ast::ParsedFile &tree);

    // Augment a global state with the name table stored in the cache.
    static void loadNameTable(GlobalState &gs, const uint8_t *const data);

    static void loadGlobalState(GlobalState &gs, const uint8_t *const data);

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
