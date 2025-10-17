#ifndef SORBET_PICKLER_H
#define SORBET_PICKLER_H

#include "common/common.h"

namespace sorbet::core::serialize {
class Pickler {
    std::vector<uint8_t> data;
    uint8_t zeroCounter = 0;

public:
    void putU4(uint32_t u);
    void putU1(const uint8_t u);
    void putS8(const int64_t i);
    void putStr(std::string_view s);
    std::vector<uint8_t> result();
    Pickler() = default;
};

// Allocator adaptor that interposes construct() calls to
// convert value initialization into default initialization.
template <typename T, typename A = std::allocator<T>> class default_init_allocator : public A {
    typedef std::allocator_traits<A> a_t;

public:
    template <typename U> struct rebind {
        using other = default_init_allocator<U, typename a_t::template rebind_alloc<U>>;
    };

    using A::A;

    template <typename U> void construct(U *ptr) noexcept(std::is_nothrow_default_constructible<U>::value) {
        ::new (static_cast<void *>(ptr)) U;
    }
    template <typename U, typename... Args> void construct(U *ptr, Args &&...args) {
        a_t::construct(static_cast<A &>(*this), ptr, std::forward<Args>(args)...);
    }
};

class UnPickler {
    int pos;
    uint8_t zeroCounter = 0;
    std::vector<uint8_t, default_init_allocator<uint8_t>> data;

public:
    uint32_t getU4();
    uint8_t getU1();
    int64_t getS8();
    std::string_view getStr();
    explicit UnPickler(const uint8_t *const compressed, spdlog::logger &tracer);
};

} // namespace sorbet::core::serialize
#endif
