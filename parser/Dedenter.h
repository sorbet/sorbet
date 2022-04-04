#ifndef SORBET_PARSER_DEDENTER_H
#define SORBET_PARSER_DEDENTER_H

#include <optional>
#include <string>

namespace sorbet::parser {

// This is defined in a Dedent.h only for testing, and is not a public API
class Dedenter final {
public:
    Dedenter(int level) : dedentLevel(level), spacesToRemove(level) {}

    std::optional<std::string> dedent(std::string_view str);

    void interrupt() {
        at_line_begin = false;
    }

private:
    void update_state(std::string_view dedented_string);

    const unsigned int dedentLevel;
    bool at_line_begin = true;
    unsigned int spacesToRemove;
};

} // namespace sorbet::parser

#endif
