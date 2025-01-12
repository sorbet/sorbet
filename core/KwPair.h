#ifndef SORBET_KWPAIR_H
#define SORBET_KWPAIR_H

#include "absl/types/span.h"

namespace sorbet::core {

template <typename PtrT> struct KwPair {
    PtrT &key;
    PtrT &value;
};

// We're going to use this type to encapsulate both the span over the kwargs,
// and also as the iterator type over that selfsame span.
template <typename PtrT> struct KwPairSpan {
    absl::Span<PtrT> span;

    KwPairSpan begin() const {
        ENFORCE(span.size() % 2 == 0);

        return KwPairSpan{span};
    }

    KwPairSpan end() const {
        ENFORCE(span.size() % 2 == 0);

        return KwPairSpan{absl::MakeSpan(span.end(), span.end())};
    }

    KwPair<PtrT> operator*() {
        return KwPair<PtrT>{span[0], span[1]};
    }

    KwPairSpan &operator++() {
        span = span.subspan(2);
        return *this;
    }

    KwPairSpan operator++(int) {
        return KwPairSpan{span.subspan(2)};
    }

    bool operator==(const KwPairSpan &other) const {
        return span.begin() == other.span.begin();
    }

    bool operator!=(const KwPairSpan &other) const {
        return span.begin() != other.span.begin();
    }
};

} // namespace sorbet::core

#endif // SORBET_KWPAIR_H
