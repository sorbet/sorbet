#ifndef SORBET_CFG_ARENA_H
#define SORBET_CFG_ARENA_H

#include "common/common.h"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <vector>

namespace sorbet::cfg {

// Bump allocator for the parts of one CFG: its instructions, its basic blocks, and the blocks' lists of bindings,
// arguments and predecessors. They are all created by the CFG's builder and freed with the CFG, so instead of one heap
// allocation and one free per part (a method has dozens of instructions, most of them 8 bytes, and a few blocks with
// three lists each) the CFG hands out pieces of a few chunks and frees the chunks at once. Destructors still run;
// only the memory is pooled. Memory a list gives back when it grows is not reused; a CFG's lists grow a few times.
class Arena final {
    static constexpr size_t FIRST_CHUNK = 1024;
    static constexpr size_t MAX_CHUNK = 64 * 1024;

    std::vector<std::unique_ptr<char[]>> chunks;
    char *cursor = nullptr;
    char *end = nullptr;
    size_t nextChunk = FIRST_CHUNK;

    void grow(size_t atLeast) {
        auto size = std::max(atLeast, nextChunk);
        chunks.emplace_back(new char[size]);
        cursor = chunks.back().get();
        end = cursor + size;
        nextChunk = std::min(nextChunk * 2, MAX_CHUNK);
    }

public:
    // What every block handed out is aligned to: enough for every instruction (see `INSN`) and every pointer-sized
    // field, checked with `static_assert` where types are placed in the arena.
    static constexpr size_t ALIGNMENT = 8;

    Arena() = default;
    // Neither copied nor moved: allocators and blocks hold pointers to the arena they came from.
    Arena(const Arena &) = delete;
    Arena &operator=(const Arena &) = delete;

    // Uninitialized memory of `bytes` bytes, aligned to `ALIGNMENT`.
    void *allocate(size_t bytes) {
        if (bytes > std::numeric_limits<size_t>::max() - ALIGNMENT) {
            throw std::bad_alloc();
        }
        bytes = (bytes + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
        if (static_cast<size_t>(end - cursor) < bytes) {
            grow(bytes);
        }
        auto *result = cursor;
        cursor += bytes;
        return result;
    }
};

// A standard allocator that takes its memory from an `Arena` and never gives it back, for the containers of a CFG.
template <class T> class ArenaAllocator {
    Arena *arena;

public:
    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    explicit ArenaAllocator(Arena &arena) noexcept : arena(&arena) {}
    template <class U> ArenaAllocator(const ArenaAllocator<U> &other) noexcept : arena(other.arenaPtr()) {}

    Arena *arenaPtr() const noexcept {
        return arena;
    }

    T *allocate(size_t n) {
        static_assert(alignof(T) <= Arena::ALIGNMENT, "the arena cannot align this type");
        return static_cast<T *>(arena->allocate(n * sizeof(T)));
    }
    void deallocate(T *, size_t) noexcept {}

    template <class U> bool operator==(const ArenaAllocator<U> &other) const noexcept {
        return arena == other.arenaPtr();
    }
    template <class U> bool operator!=(const ArenaAllocator<U> &other) const noexcept {
        return arena != other.arenaPtr();
    }
};

} // namespace sorbet::cfg

#endif
