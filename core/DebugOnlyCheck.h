#ifndef RUBY_TYPER_DEBUGONLY_H
#define RUBY_TYPER_DEBUGONLY_H
#include "common/common.h"
namespace sorbet::core {

template <class T, bool DebugMode = debug_mode> class DebugOnlyCheck;

template <class T> class DebugOnlyCheck<T, true> {
private:
    T storage;

public:
    template <typename... Args> constexpr DebugOnlyCheck(Args &&... args) : storage(std::forward<Args>(args)...) {}

    template <typename... Args> void runDebugOnlyCheck(Args &&... args) const {
        storage.check(std::forward<Args>(args)...);
    }
};

template <class T> class DebugOnlyCheck<T, false> {
public:
    template <typename... Args> constexpr DebugOnlyCheck(Args &&... args) {}

    template <typename... Args> void runDebugOnlyCheck(Args &&... args) const {}
};

}; // namespace sorbet::core
#endif
