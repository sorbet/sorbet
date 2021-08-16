#ifndef RUBY_TYPER_DEBUGONLY_H
#define RUBY_TYPER_DEBUGONLY_H
#include "common/common.h"
namespace sorbet::core {

/*
 * DebugOnlyCheck is an abstraction used to simplify the creation
 * of types which carry additional metadata used for sanity
 * checking in debug builds only.
 *
 * In debug builds, a DebugOnlyCheck<T> is a thin wrapper around a
 * `T`, which forwards its constructor to T's, and whose
 * `runDebugOnlyCheck(args...)` method forwards to
 * `T.check(args...)`.
 *
 * In optimized release builds, `DebugOnlyCheck<T>` is empty, and
 * should consume no storage and be entirely compiled out at
 * runtime.
 *
 * HOWEVER: In C++ prior to C++20, the size of any object is
 * guaranteed to be at least one byte, meaning that
 * `DebugOnlyCheck<T>` instances will still consume storage. Users
 * of this class are intended to take advantage of the Empty Base
 * Optimization to get around this behavior, by inheriting from
 * `EmptyBaseClass<T>` instead of including one as a member. See
 * `NameRef` and `NameData` for examples of this usage.
 *
 * Once we're building using C++20, we can optionally switch to
 * using the `no_unique_address` annotation on `DebugOnlyCheck`
 * instances, instead of inheriting from them.
 *
 * See https://en.cppreference.com/w/cpp/language/ebo for more
 * details on zero-side objects and the Empty Base Optimization
 */

template <class T, bool DebugMode = debug_mode> class DebugOnlyCheck;

template <class T> class DebugOnlyCheck<T, true> {
private:
    T storage;

public:
    template <typename... Args> constexpr DebugOnlyCheck(Args &&...args) : storage(std::forward<Args>(args)...) {}

    template <typename... Args> void runDebugOnlyCheck(Args &&...args) const {
        storage.check(std::forward<Args>(args)...);
    }
};

template <class T> class DebugOnlyCheck<T, false> {
public:
    template <typename... Args> constexpr DebugOnlyCheck(Args &&...args) {}

    template <typename... Args> void runDebugOnlyCheck(Args &&...args) const {}
};

}; // namespace sorbet::core
#endif
