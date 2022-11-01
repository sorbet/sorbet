#ifndef SORBET_ENFORCENOTIMER_H
#define SORBET_ENFORCENOTIMER_H

#include "common/exception/Exception.h"
#include "common/os/os.h"
#include "sorbet_version/sorbet_version.h"

#define _MAYBE_ADD_COMMA(...) , ##__VA_ARGS__

// A faster version of ENFORCE that does not emit a timer. Useful for checks that happen extremely frequently and
// are O(1). Please avoid using unless ENFORCE shows up in profiles.
#define ENFORCE_NO_TIMER(x, ...)                                                                            \
    do {                                                                                                    \
        if (::sorbet::debug_mode) {                                                                         \
            if (!(x)) {                                                                                     \
                ::sorbet::Exception::failInFuzzer();                                                        \
                if (stopInDebugger()) {                                                                     \
                    (void)!(x);                                                                             \
                }                                                                                           \
                ::sorbet::Exception::enforce_handler(#x, __FILE__, __LINE__ _MAYBE_ADD_COMMA(__VA_ARGS__)); \
            }                                                                                               \
        }                                                                                                   \
    } while (false);

#endif // SORBET_ENFORCENOTIMER_H
