#ifndef SORBET_POLARITY_H
#define SORBET_POLARITY_H

#include <string>

namespace sorbet::core {

enum class Variance {
    CoVariant = 1,
    ContraVariant = -1,
    Invariant = 0,
};

enum class Polarity {
    Positive = 1,
    Neutral = 0,
    Negative = -1,
};

class Polarities final {
public:
    // Show a Polarity using the same in/out notation as Variance.
    static std::string showPolarity(const Polarity polarity) {
        switch (polarity) {
            case Polarity::Positive:
                return ":out";
            case Polarity::Neutral:
                return "invariant";
            case Polarity::Negative:
                return ":in";
        }
    }

    static Polarity negatePolarity(const Polarity polarity) {
        switch (polarity) {
            case Polarity::Positive:
                return Polarity::Negative;
            case Polarity::Neutral:
                return Polarity::Neutral;
            case Polarity::Negative:
                return Polarity::Positive;
        }
    }

    static std::string showVariance(const core::Variance variance) {
        switch (variance) {
            case core::Variance::CoVariant:
                return ":out";
            case core::Variance::Invariant:
                return "invariant";
            case core::Variance::ContraVariant:
                return ":in";
        }
    }

    static bool hasCompatibleVariance(const Polarity polarity, const core::Variance argVariance) {
        switch (polarity) {
            case Polarity::Positive:
                return argVariance != core::Variance::ContraVariant;
            case Polarity::Neutral:
                return true;
            case Polarity::Negative:
                return argVariance != core::Variance::CoVariant;
        }
    }
};

} // namespace sorbet::core

#endif
