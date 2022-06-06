#ifndef SORBET_ARITY_HASH_H
#define SORBET_ARITY_HASH_H

#include "common/common.h"

namespace sorbet::core {

class ArityHash {
    // For use with isDefined
    constexpr static uint32_t UNDEFINED = 0;
    // In case something naturally hashed to a value of `0`, we force it to collide with things that
    // hashed to `1` instead, to reserve `0` as a special constant (checking for isDefined())
    constexpr static uint32_t UNDEFINED_COLLISION_AVOID = 1;

    // To be able to implement isAliasMethod
    //
    // Asking for the arity of an alias method (not the arity of the method it dealiases to) doesn't
    // make sense, as there are no parameters in an alias method definition.
    constexpr static uint32_t ALIAS_METHOD = 2;
    // Same as UNDEFINED_COLLISION_AVOID above, but for alias methods.
    constexpr static uint32_t ALIAS_METHOD_COLLISION_AVOID = 3;

public:
    ArityHash() noexcept : _hashValue(0) {}
    explicit ArityHash(uint32_t _hashValue);
    static ArityHash aliasMethodHash();

    inline bool isDefined() const {
        return _hashValue != UNDEFINED;
    }
    inline bool isAliasMethod() const {
        ENFORCE(isDefined());
        return _hashValue == ALIAS_METHOD;
    }

    inline bool operator==(const ArityHash &rhs) const noexcept {
        ENFORCE(isDefined());
        ENFORCE(rhs.isDefined());
        return _hashValue == rhs._hashValue;
    }

    inline bool operator!=(const ArityHash &rhs) const noexcept {
        return !(rhs == *this);
    }

    inline bool operator<(const ArityHash &rhs) const noexcept {
        return this->_hashValue < rhs._hashValue;
    }

    // Pseudo-private, only public for serialization
    uint32_t _hashValue;
};

} // namespace sorbet::core

#endif
