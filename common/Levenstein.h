#ifndef RUBY_TYPER_LEVENSTEIN_H
#define RUBY_TYPER_LEVENSTEIN_H
#include "absl/strings/string_view.h"

namespace ruby_typer {

class Levenstein {
public:
    static int distance(absl::string_view s1, absl::string_view s2, int bound);
};

} // namespace ruby_typer
#endif // RUBY_TYPER_LEVENSTEIN_H
