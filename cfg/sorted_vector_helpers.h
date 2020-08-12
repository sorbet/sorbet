#ifndef SORBET_SORTED_VECTOR_HELPERS_H
#define SORBET_SORTED_VECTOR_HELPERS_H

#include <vector>

namespace sorbet::cfg {
// Given sorted vectors `data` and `toRemove`, removes `toRemove` from `data`.
void removeFrom(std::vector<int> &data, const std::vector<int> &toRemove);

// Merges sorted vectors `info` and `from` (which contain no duplicate entries) without introducing duplicates (set
// union).
void setMerge(std::vector<int> &info, const std::vector<int> &from);
} // namespace sorbet::cfg

#endif // SORBET_SORTED_VECTOR_HELPERS_H
