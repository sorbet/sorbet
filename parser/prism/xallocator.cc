#include "parser/prism/PrismArena.h"
#include "parser/prism/prism_xallocator.h"

#include "common/common.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>

namespace sorbet::parser::Prism {

namespace {

// The 16-byte header in front of every block handed out by this file.
struct BlockHeader {
    uint64_t size;
    uint64_t tag;
};
static_assert(sizeof(BlockHeader) == PrismArena::HEADER_SIZE);

constexpr uint64_t ARENA_TAG = 0x5052534d41524e41;  // "PRSMARNA"
constexpr uint64_t SYSTEM_TAG = 0x5052534d53595354; // "PRSMSYST"

// Larger requests are refused (returned as `NULL`, like `malloc`) rather than allowed to overflow the size arithmetic
// below. Prism's requests are proportional to the source being parsed.
constexpr size_t MAX_REQUEST = std::numeric_limits<size_t>::max() / 4;

thread_local PrismArena *currentArena = nullptr;
thread_local uint64_t foreignAllocationCount = 0;

// The header of a block this file handed out, or `nullptr` for a pointer that did not come from here (memory that some
// caller allocated with the system allocator and then handed to Prism, which frees it through these hooks). Every
// Prism-owned allocation is meant to come through here; a debug build says so, a release build lets the block go back
// to the system allocator it came from.
BlockHeader *headerOf(void *block) {
    auto *header = reinterpret_cast<BlockHeader *>(static_cast<char *>(block) - PrismArena::HEADER_SIZE);
    if (header->tag == ARENA_TAG || header->tag == SYSTEM_TAG) {
        return header;
    }
    ENFORCE_NO_TIMER(false, "Prism freed a block that its allocator hooks did not hand out");
    return nullptr;
}

void *systemAllocate(size_t size) {
    if (size > MAX_REQUEST) {
        return nullptr;
    }
    auto *header = static_cast<BlockHeader *>(std::malloc(size + PrismArena::HEADER_SIZE));
    if (header == nullptr) {
        return nullptr;
    }
    header->size = size;
    header->tag = SYSTEM_TAG;
    foreignAllocationCount++;
    return header + 1;
}

} // namespace

void PrismArena::grow(size_t needed) {
    auto size = std::max(needed, nextChunkSize);
    nextChunkSize = std::min(nextChunkSize * 2, MAX_CHUNK_SIZE);
    chunks.push_back(Chunk{std::unique_ptr<char[]>(new char[size]), size});
    cursor = chunks.back().memory.get();
    limit = cursor + size;
}

void *PrismArena::allocate(size_t size) {
    ENFORCE_NO_TIMER(size <= MAX_REQUEST);
    // Keep every block 16-byte aligned: the header is 16 bytes and each chunk starts aligned.
    auto total = ((size + 15) & ~static_cast<size_t>(15)) + HEADER_SIZE;
    if (static_cast<size_t>(limit - cursor) < total) {
        grow(total);
    }
    auto *header = reinterpret_cast<BlockHeader *>(cursor);
    header->size = size;
    header->tag = ARENA_TAG;
    cursor += total;
    return header + 1;
}

uint64_t PrismArena::foreignAllocations() {
    return foreignAllocationCount;
}

ArenaScope::ArenaScope(PrismArena &arena) : previous(currentArena) {
    currentArena = &arena;
}

ArenaScope::~ArenaScope() {
    currentArena = previous;
}

} // namespace sorbet::parser::Prism

using sorbet::parser::Prism::ARENA_TAG;
using sorbet::parser::Prism::currentArena;
using sorbet::parser::Prism::headerOf;
using sorbet::parser::Prism::MAX_REQUEST;
using sorbet::parser::Prism::PrismArena;
using sorbet::parser::Prism::systemAllocate;

extern "C" {

void *sorbet_prism_xmalloc(size_t size) {
    if (currentArena == nullptr) {
        return systemAllocate(size);
    }
    if (size > MAX_REQUEST) {
        return nullptr;
    }
    try {
        return currentArena->allocate(size);
    } catch (const std::bad_alloc &) {
        // Out of memory is reported the way `malloc` reports it; an exception must not cross Prism's C frames.
        return nullptr;
    }
}

void *sorbet_prism_xcalloc(size_t count, size_t size) {
    size_t total;
    if (__builtin_mul_overflow(count, size, &total)) {
        return nullptr;
    }
    auto *block = sorbet_prism_xmalloc(total);
    if (block != nullptr) {
        std::memset(block, 0, total);
    }
    return block;
}

void *sorbet_prism_xrealloc(void *ptr, size_t size) {
    if (ptr == nullptr) {
        return sorbet_prism_xmalloc(size);
    }
    auto *header = headerOf(ptr);
    if (header == nullptr) {
        return std::realloc(ptr, size);
    }
    if (size > MAX_REQUEST) {
        return nullptr;
    }
    if (currentArena == nullptr && header->tag != ARENA_TAG) {
        // System block, no arena to move it to: let the system allocator resize it in place if it can.
        auto *resized = static_cast<char *>(std::realloc(header, size + PrismArena::HEADER_SIZE));
        if (resized == nullptr) {
            return nullptr;
        }
        reinterpret_cast<uint64_t *>(resized)[0] = size;
        return resized + PrismArena::HEADER_SIZE;
    }
    // Arena blocks are never resized in place (the old bytes stay in the arena until it goes away); a system block
    // moving into an arena is copied and released.
    auto *block = sorbet_prism_xmalloc(size);
    if (block == nullptr) {
        return nullptr;
    }
    std::memcpy(block, ptr, std::min<size_t>(header->size, size));
    if (header->tag != ARENA_TAG) {
        std::free(header);
    }
    return block;
}

void sorbet_prism_xfree(void *ptr) {
    if (ptr == nullptr) {
        return;
    }
    auto *header = headerOf(ptr);
    if (header == nullptr) {
        std::free(ptr);
    } else if (header->tag != ARENA_TAG) {
        std::free(header);
    }
}

} // extern "C"
