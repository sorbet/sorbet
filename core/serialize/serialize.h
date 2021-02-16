#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::core::serialize {
struct CachedFile {
    std::shared_ptr<core::File> file;
    ast::ExpressionPtr tree;
};

class Serializer {
public:
    static const u4 VERSION = 6;

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

    // Loads the given file and its AST.
    static CachedFile loadFile(const GlobalState &gs, const u1 *const data);
};
}; // namespace sorbet::core::serialize

#endif
