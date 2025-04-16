#ifndef SORBET_LEVENSTEIN_H
#define SORBET_LEVENSTEIN_H
#include <string_view>
#include <vector>

namespace sorbet {

class Levenstein {
    std::vector<int> column;

public:
    int distance(std::string_view s1, std::string_view s2, int bound) noexcept;
};

} // namespace sorbet
#endif // SORBET_LEVENSTEIN_H
