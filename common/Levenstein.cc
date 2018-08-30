#include "common/Levenstein.h"
#include "absl/algorithm/container.h"
#include <numeric> // iota
#include <vector>

using namespace std;

int sorbet::Levenstein::distance(absl::string_view s1, absl::string_view s2, int bound) noexcept {
    if (s1.data() == s2.data() && s1.size() == s2.size()) {
        return 0;
    }
    // A mildly tweaked version from
    // https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++
    int s1len = s1.size();
    int s2len = s2.size();
    if (s2len < s1len) {
        swap(s1, s2);
        swap(s1len, s2len);
    }

    if (s2len - s1len > bound) {
        return INT_MAX;
    }

    vector<int> column(s1len + 1);
    absl::c_iota(column, 0);

    for (int x = 1; x <= s2len; x++) {
        column[0] = x;
        int last_diagonal = x - 1;
        for (auto y = 1; y <= s1len; y++) {
            int old_diagonal = column[y];
            auto possibilities = {column[y] + 1, column[y - 1] + 1, last_diagonal + (s1[y - 1] == s2[x - 1] ? 0 : 1)};
            column[y] = min(possibilities);
            last_diagonal = old_diagonal;
        }
    }
    int result = column[s1len];
    return result;
}
