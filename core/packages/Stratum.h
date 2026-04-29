#ifndef SORBET_CORE_PACKAGES_STRATUM_H
#define SORBET_CORE_PACKAGES_STRATUM_H

#include <cstdint>

namespace sorbet::core::packages {

// The index of a group of packages in the condensation graph traversal.
class Stratum {
    // We use uint16_t here, even though that might seem small, as overflowing a uint16_t would require a non-cyclic
    // chain of 65535 dependencies in the package graph. At time of writing (2026-04-29), the largest stratum we've
    // observed has been less than 200, so hopefully we won't need to change this for a long time.
    uint16_t stratum = 0;

public:
    Stratum() = default;
    constexpr explicit Stratum(uint16_t stratum) : stratum{stratum} {}

    // Fetch the raw id
    constexpr uint16_t rawId() const {
        return this->stratum;
    }

    constexpr bool operator==(Stratum other) const {
        return this->stratum == other.stratum;
    }

    constexpr bool operator!=(Stratum other) const {
        return this->stratum != other.stratum;
    }

    constexpr bool operator<(Stratum other) const {
        return this->stratum < other.stratum;
    }

    constexpr bool operator<=(Stratum other) const {
        return this->stratum <= other.stratum;
    }

    constexpr bool operator>(Stratum other) const {
        return this->stratum > other.stratum;
    }

    constexpr bool operator>=(Stratum other) const {
        return this->stratum >= other.stratum;
    }
};

} // namespace sorbet::core::packages

#endif
