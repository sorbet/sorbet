#ifndef SORBET_PARSER_PRISM_ARENA_H
#define SORBET_PARSER_PRISM_ARENA_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace sorbet::parser::Prism {

// The memory of one parse. Prism allocates a node, a node list or a string for nearly every token, and frees them one
// by one when the tree is destroyed; an arena hands them out from a few large chunks and frees everything at once when
// the parse result goes away, which is the only time Prism memory is freed anyway.
//
// Every block carries a 16-byte header with its size and a tag saying whether it came from an arena or from the system
// allocator, so `sorbet_prism_xfree` and `sorbet_prism_xrealloc` do the right thing for a block no matter where they
// are called from. Blocks are handed out from the arena only while an `ArenaScope` for it is alive on the current
// thread (see Parser); Prism calls made outside any scope get system memory, and `foreignAllocations` counts them so a
// caller can tell whether a parse's tree consists of arena blocks only.
class PrismArena final {
    struct Chunk {
        std::unique_ptr<char[]> memory;
        size_t size;
    };
    std::vector<Chunk> chunks;
    char *cursor = nullptr;
    char *limit = nullptr;
    size_t nextChunkSize;

    void grow(size_t needed);

public:
    static constexpr size_t HEADER_SIZE = 16;
    static constexpr size_t FIRST_CHUNK_SIZE = 128 * 1024;
    static constexpr size_t MAX_CHUNK_SIZE = 8 * 1024 * 1024;

    PrismArena() : nextChunkSize(FIRST_CHUNK_SIZE) {}
    PrismArena(const PrismArena &) = delete;
    PrismArena &operator=(const PrismArena &) = delete;

    // A block of `size` bytes, 16-byte aligned, preceded by its header. Never returns null.
    void *allocate(size_t size);

    // Number of Prism allocations on this thread that were not served by an arena (no scope was active).
    static uint64_t foreignAllocations();
};

// Makes `arena` the destination of Prism's allocations on this thread for the scope's lifetime.
class ArenaScope final {
    PrismArena *previous;

public:
    explicit ArenaScope(PrismArena &arena);
    ~ArenaScope();
    ArenaScope(const ArenaScope &) = delete;
    ArenaScope &operator=(const ArenaScope &) = delete;
};

} // namespace sorbet::parser::Prism

#endif // SORBET_PARSER_PRISM_ARENA_H
