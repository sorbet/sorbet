#ifndef SORBET_COMMON_SORT_HPP
#define SORBET_COMMON_SORT_HPP

#include "pdqsort.h"

template <class Container, class Compare> inline void fast_sort(Container &container, Compare &&comp) {
    pdqsort(container.begin(), container.end(), std::forward<Compare>(comp));
};

template <class Container> inline void fast_sort(Container &container) {
    pdqsort(container.begin(), container.end());
};

// You should prefer using fast_sort to this.  This function exists for the case where
// you don't want to sort the entire container and to centralize the "hiding" of
// pdqsort to this header.
template <class Iterator> inline void fast_sort_range(Iterator start, Iterator end) {
    pdqsort(start, end);
}

#endif
