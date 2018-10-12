#ifndef RUBY_TYPER_DEBUGONLY_H
#define RUBY_TYPER_DEBUGONLY_H
#include "common/common.h"
namespace sorbet::core {

template <class T> class DebugOnlyCheck {
private:
#ifdef DEBUG_MODE
    T storage;
#endif

public:
    template <typename... Args>
    DebugOnlyCheck(Args &&... args)
#ifdef DEBUG_MODE
        : storage(std::forward<Args>(args)...)
#endif
    {
    }

    template <typename... Args> void runDebugOnlyCheck(Args &&... args) const {
#ifdef DEBUG_MODE
        storage.check(std::forward<Args>(args)...);
#endif
    }
};

}; // namespace sorbet::core
#endif
