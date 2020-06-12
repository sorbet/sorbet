#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::core::serialize {
struct CachedFile {
    std::shared_ptr<core::File> file;
    ast::TreePtr tree;
};

class Serializer {
public:
    static const u4 VERSION = 5;
    static const u1 GLOBAL_STATE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown
    static const u1 FILE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown

    // Serialize a global state.
    static std::vector<u1> store(GlobalState &gs);

    // Stores a GlobalState, but only includes `File`s with Type == Payload.
    // This can be used in conjunction with `storeFile` to store
    // a global state containing a name table along side a large number of
    // individual cached files, which can be loaded independently.
    static std::vector<u1> storePayloadAndNameTable(GlobalState &gs);

    // Serializes a file and its AST.
    static std::vector<u1> storeFile(const core::File &file, ast::ParsedFile &tree);

    static void loadGlobalState(GlobalState &gs, const u1 *const data);

    static u4 loadGlobalStateUUID(const GlobalState &gs, const u1 *const data);

    // Loads the given file and its AST. Overwrites file references in the AST with the given file ref.
    static CachedFile loadFile(const GlobalState &gs, core::FileRef fref, const u1 *const data);
};
}; // namespace sorbet::core::serialize

#endif
