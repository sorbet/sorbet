#include "common/Levenstein.h"
#include "common/common.h"
#include <vector>

using namespace std;

template <bool sensitive>
sorbet::Levenstein::CasedDistanceResults sorbet::Levenstein::distance(string_view s1, string_view s2,
                                                                      int bound) noexcept {
    if (s1.data() == s2.data() && s1.size() == s2.size()) {
        return CasedDistanceResults{0, 0};
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
        return CasedDistanceResults{INT_MAX, INT_MAX};
    }

    vector<int> sensitiveColumn(s1len + 1);
    vector<int> insensitiveColumn(s1len + 1);

    absl::c_iota(sensitiveColumn, 0);

    if (!sensitive) {
        absl::c_iota(insensitiveColumn, 0);
    }

    for (int x = 1; x <= s2len; x++) {
        sensitiveColumn[0] = x;

        if (!sensitive) {
            insensitiveColumn[0] = x;
        }

        int sensitiveLastDiagonal = x - 1;
        int insensitiveLastDiagonal = x - 1;

        for (auto y = 1; y <= s1len; y++) {
            int sensitiveOldDiagonal = sensitiveColumn[y];

            auto sensitivePossibilities = {sensitiveColumn[y] + 1, sensitiveColumn[y - 1] + 1,
                                           sensitiveLastDiagonal + (s1[y - 1] == s2[x - 1] ? 0 : 1)};

            sensitiveColumn[y] = min(sensitivePossibilities);
            sensitiveLastDiagonal = sensitiveOldDiagonal;

            if (!sensitive) {
                int insensitiveOldDiagonal = insensitiveColumn[y];
                auto insensitivePossibilities = {
                    insensitiveColumn[y] + 1, insensitiveColumn[y - 1] + 1,
                    insensitiveLastDiagonal +
                        (std::tolower(s1[y - 1], std::locale()) == std::tolower(s2[x - 1], std::locale()) ? 0 : 1)};
                insensitiveColumn[y] = min(insensitivePossibilities);
                insensitiveLastDiagonal = insensitiveOldDiagonal;
            }
        }
    }
    int sensitiveResult = sensitiveColumn[s1len];
    if (sensitive) {
        return CasedDistanceResults{.sensitive = sensitiveResult};
    } else {
        int insensitiveResult = insensitiveColumn[s1len];
        return CasedDistanceResults{.sensitive = sensitiveResult, .insensitive = insensitiveResult};
    }
}

template <>
sorbet::Levenstein::CasedDistanceResults sorbet::Levenstein::distance<true>(string_view s1, string_view s2,
                                                                            int bound) noexcept;

template <>
sorbet::Levenstein::CasedDistanceResults sorbet::Levenstein::distance<false>(string_view s1, string_view s2,
                                                                             int bound) noexcept;