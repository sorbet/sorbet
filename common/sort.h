#ifndef SORBET_COMMON_SORT_HPP
#define SORBET_COMMON_SORT_HPP

#include "pdqsort.h"

template <class Container, class Compare> inline void fast_sort(Container &container, Compare &&comp) {
    pdqsort(container.begin(), container.end(), std::forward<Compare>(comp));
};

template <class Container> inline void fast_sort(Container &container) {
    pdqsort(container.begin(), container.end());
};

#endif
