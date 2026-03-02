#ifndef RUBY_PARSER_DIAGNOSTIC_HH
#define RUBY_PARSER_DIAGNOSTIC_HH

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "diagnostic_class.hh"
#include "location.hh"
#include "token.hh"

namespace ruby_parser {

enum class dlevel {
    NOTE = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4,
};

class diagnostic {
public:
    struct range {
        size_t beginPos;
        size_t endPos;

        range(size_t beginPos, size_t endPos) : beginPos(beginPos), endPos(endPos) {}
        explicit range(token_t token) : beginPos(token->start()), endPos(token->end()) {}
        explicit range(location loc) : beginPos(loc.beginPos()), endPos(loc.endPos()) {}

        bool operator==(const range &rhs) const {
            return this->beginPos == rhs.beginPos && this->endPos == rhs.endPos;
        }
    };

private:
    dlevel level_;
    dclass type_;
    range location_;
    std::string data_;
    std::optional<range> extra_location_;

public:
    diagnostic(dlevel lvl, dclass type, range location, const std::string &data = "",
               std::optional<range> extra_location = std::nullopt)
        : level_(lvl), type_(type), location_(location), data_(data), extra_location_(extra_location) {}

    diagnostic(dlevel lvl, dclass type, const token_t token, const std::string &data = "",
               const token_t extra_token = nullptr)
        : diagnostic(lvl, type, range(token), data,
                     extra_token != nullptr ? std::make_optional<range>(range(extra_token)) : std::nullopt) {}

    dlevel level() const {
        return level_;
    }

    dclass error_class() const {
        return type_;
    }

    const std::string &data() const {
        return data_;
    }

    const range &location() const {
        return location_;
    }

    const std::optional<range> &extra_location() const {
        return extra_location_;
    }

    void set_extra_location(range extra_location) {
        this->extra_location_ = extra_location;
    }
};

using diagnostics_t = std::vector<diagnostic>;

} // namespace ruby_parser

#endif
