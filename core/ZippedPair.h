#ifndef SORBET_ZIPPED_PAIR_H
#define SORBET_ZIPPED_PAIR_H

#include "absl/types/span.h"

namespace sorbet::core {

template <typename KeyT, typename ValueT> struct ZipPair {
    KeyT &key;
    ValueT &value;
};

// We're going to use this type to encapsulate both the span over the args,
// and also as the iterator type over that selfsame span.
template <typename KeyT, typename ValueT> struct ZippedPairSpan {
    absl::Span<KeyT> keys;
    absl::Span<ValueT> values;

    ZippedPairSpan begin() const {
        return ZippedPairSpan{keys, values};
    }

    ZippedPairSpan end() const {
        ENFORCE(keys.size() == values.size());
        return ZippedPairSpan{absl::MakeSpan(keys.end(), keys.end()), absl::MakeSpan(values.end(), values.end())};
    }

    ZipPair<KeyT, ValueT> operator*() {
        return ZipPair<KeyT, ValueT>{keys[0], values[0]};
    }

    ZippedPairSpan &operator++() {
        keys = keys.subspan(1);
        values = values.subspan(1);
        return *this;
    }

    ZippedPairSpan operator++(int) {
        ZippedPairSpan copy(*this);
        ++*this;
        return copy;
    }

    // The equality operators only check span equality of the keys; this is not techically
    // correct, but the loops that use this were previously using only the keys for
    // controlling the iteration anyway.  Doing a single check is also slightly more
    // efficient; we check in debug mode for an extra measure of safety.
    bool operator==(const ZippedPairSpan &other) const {
        bool equal = keys.begin() == other.keys.begin();
        ENFORCE(equal == (values.begin() == other.values.begin()));
        return equal;
    }

    bool operator!=(const ZippedPairSpan &other) const {
        bool equal = keys.begin() != other.keys.begin();
        ENFORCE(equal == (values.begin() != other.values.begin()));
        return equal;
    }
};

} // namespace sorbet::core

#endif // SORBET_ZIPPED_PAIR_H
