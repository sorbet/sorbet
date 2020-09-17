#ifndef SORBET_SORTED_VECTOR_HELPERS_H
#define SORBET_SORTED_VECTOR_HELPERS_H

#include <vector>

namespace sorbet::cfg {
class SortedVectorHelpers final {
public:
    // Given sorted vectors `data` and `toRemove` (with no duplicate entries), removes items in `toRemove` from `data`.
    static void setDifferenceInplace(std::vector<int> &data, const std::vector<int> &toRemove);

    // Merges sorted vectors `info` and `from` (which contain no duplicate entries) without introducing duplicates (set
    // union).
    static void setUnionInplace(std::vector<int> &info, const std::vector<int> &from);
};
} // namespace sorbet::cfg

#endif // SORBET_SORTED_VECTOR_HELPERS_H
