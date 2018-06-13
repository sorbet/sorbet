#ifndef SORBET_LEVENSTEIN_H
#define SORBET_LEVENSTEIN_H
#include "absl/strings/string_view.h"

namespace sorbet {

class Levenstein {
public:
    static int distance(absl::string_view s1, absl::string_view s2, int bound) noexcept;
};

} // namespace sorbet
#endif // SORBET_LEVENSTEIN_H
