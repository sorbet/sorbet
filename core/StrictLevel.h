#ifndef SORBET_CORE_STRICT_LEVEL_H
#define SORBET_CORE_STRICT_LEVEL_H

namespace sorbet {
namespace core {
enum class StrictLevel {
    Stripe = 0, // Temporary; A level defined as "whatever Stripe needs it to be
                // right now". Eventually this will be named "Ruby" and contain even fewer checks.
    Typed = 1,
    Strict = 2,
    Strong = 3,
};
}
} // namespace sorbet

#endif
