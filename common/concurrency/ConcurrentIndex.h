#ifndef SORBET_CONCURRENCY_CONCURRENT_INDEX_H
#define SORBET_CONCURRENCY_CONCURRENT_INDEX_H

#include <atomic>
#include <optional>

namespace sorbet {

// An atomic counter, up to a given limit. Aligned on a 64-byte boundary to ensure that it's in its own cache line, to
// minimize any unintended synchronization between other threads.
class alignas(64) ConcurrentIndex {
    std::atomic<size_t> _next = 0;
    const size_t _size;

public:
    ConcurrentIndex(size_t size) : _size{size} {}

    // Fetch the next available index, returning `std::nullopt` if there are no more available.
    std::optional<size_t> next() {
        auto val = this->_next.fetch_add(1);
        return val < this->_size ? std::make_optional(val) : std::nullopt;
    }

    // Returns how many indices have been consumed.
    size_t doneEstimate() {
        auto consumed = this->_next.load(std::memory_order_relaxed);
        return consumed < this->_size ? consumed : this->_size;
    }
};

} // namespace sorbet

#endif
