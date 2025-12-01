#ifndef SORBET_ZIPPED_PAIR_H
#define SORBET_ZIPPED_PAIR_H

#include "absl/types/span.h"

#include <iterator>

namespace sorbet::core {

template <typename KeyT, typename ValueT> struct ZipPair {
    KeyT &key;
    ValueT &value;
};

template <typename KeyT, typename ValueT> struct ZippedPairIterator {
    absl::Span<KeyT> keys;
    absl::Span<ValueT> values;

    using value_type = ZipPair<KeyT, ValueT>;
    using difference_type = void;
    using pointer = ZipPair<KeyT, ValueT> *;
    using reference = ZipPair<KeyT, ValueT> &;
    using iterator_category = std::forward_iterator_tag;

    ZipPair<KeyT, ValueT> operator*() {
        return ZipPair<KeyT, ValueT>{keys[0], values[0]};
    }

    ZippedPairIterator &operator++() {
        keys = keys.subspan(1);
        values = values.subspan(1);
        return *this;
    }

    ZippedPairIterator operator++(int) {
        ZippedPairIterator copy(*this);
        ++*this;
        return copy;
    }

    // The equality operators only check span equality of the keys; this is not techically
    // correct, but the loops that use this were previously using only the keys for
    // controlling the iteration anyway.  Doing a single check is also slightly more
    // efficient; we check in debug mode for an extra measure of safety.
    bool operator==(const ZippedPairIterator &other) const {
        bool equal = keys.begin() == other.keys.begin();
        ENFORCE(equal == (values.begin() == other.values.begin()));
        return equal;
    }

    bool operator!=(const ZippedPairIterator &other) const {
        bool equal = keys.begin() != other.keys.begin();
        ENFORCE(equal == (values.begin() != other.values.begin()));
        return equal;
    }
};

template <typename KeyT, typename ValueT> struct ZippedPairSpan {
    absl::Span<KeyT> keys;
    absl::Span<ValueT> values;

    using iterator = ZippedPairIterator<KeyT, ValueT>;
    using const_iterator = ZippedPairIterator<const KeyT, const ValueT>;

    iterator begin() const {
        return iterator{keys, values};
    }

    iterator end() const {
        ENFORCE(keys.size() == values.size());
        return iterator{absl::MakeSpan(keys.end(), keys.end()), absl::MakeSpan(values.end(), values.end())};
    }
};

} // namespace sorbet::core

#endif // SORBET_ZIPPED_PAIR_H
