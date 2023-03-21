#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::core::serialize {
class Serializer {
public:
    static const uint32_t VERSION = 6;

    // Serialize a global state.
    static std::vector<uint8_t> store(const GlobalState &gs);

    // Stores a GlobalState, but only includes `File`s with Type == Payload.
    // This can be used in conjunction with `storeFile` to store
    // a global state containing a name table along side a large number of
    // individual cached files, which can be loaded independently.
    static std::vector<uint8_t> storePayloadAndNameTable(const GlobalState &gs);

    // Serializes an AST and file hash.
    static std::vector<uint8_t> storeTree(const core::File &file, const ast::ParsedFile &tree);

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
