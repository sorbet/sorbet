#ifndef SORBET_SERIALIZE_H
#define SORBET_SERIALIZE_H
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet {
namespace core {
namespace serialize {
class Serializer {
public:
    static const u4 VERSION = 4;
    static const u1 GLOBAL_STATE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown
    static const u1 FILE_COMPRESSION_DEGREE =
        10; // >20 introduce decompression slowdown, >10 introduces compression slowdown

    // Serialize a global state
    static std::vector<u1> store(GlobalState &gs);

    // Stores a GlobalState, but only includes `File`s with Type ==
    // Payload. This can be used in conjunction with `storeExpression` to store
    // a global state containing a name table along side a large number of
    // individual cached files, which can be loaded independently.
    static std::vector<u1> storePayloadAndNameTable(GlobalState &gs);
    static std::vector<u1> storeExpression(GlobalState &gs, std::unique_ptr<ast::Expression> &e);

    // Loads an ast::Expression saved by storeExpression. Optionally overrides
    // the saved file ID to the caller-specified ID.
    static std::unique_ptr<ast::Expression> loadExpression(GlobalState &gs, const u1 *const p, u4 forceId = 0);
    static void loadGlobalState(GlobalState &gs, const u1 *const data);
};
} // namespace serialize
} // namespace core
}; // namespace sorbet

#endif
