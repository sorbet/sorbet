#ifndef SRUBY_REPORTER_H
#define SRUBY_REPORTER_H

#include "Loc.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>
#include <memory>

namespace ruby_typer {
namespace core {

class ErrorClass {
public:
    u2 code;

    constexpr ErrorClass(u2 code) : code(code){};
    ErrorClass(const ErrorClass &rhs) = default;
    ErrorClass &operator=(const ErrorClass &rhs) = default;

    bool operator==(const ErrorClass &rhs) const {
        return code == rhs.code;
    }

    bool operator!=(const ErrorClass &rhs) const {
        return !(*this == rhs);
    }
};

struct BasicError {
    Loc loc;
    ErrorClass what;
    std::string formatted;
    bool isCritical;

    BasicError(Loc loc, ErrorClass what, std::string formatted);

    virtual std::string toString(const GlobalState &gs);
    virtual ~BasicError() = default;
    static std::string filePosToString(const GlobalState &gs, Loc loc);
};

struct ErrorLine {
    Loc loc;
    std::string formattedMessage;
    ErrorLine(Loc loc, std::string formattedMessage) : loc(loc), formattedMessage(formattedMessage){};

    template <typename... Args> static ErrorLine from(Loc loc, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        return ErrorLine(loc, formatted);
    }
    std::string toString(const GlobalState &gs);
};

struct ErrorSection {
    std::string header;
    std::vector<ErrorLine> messages;
    ErrorSection(std::string header) : header(header) {}
    ErrorSection(std::string header, std::initializer_list<ErrorLine> messages) : header(header), messages(messages) {}
    ErrorSection(std::string header, std::vector<ErrorLine> messages) : header(header), messages(messages) {}
    ErrorSection(std::initializer_list<ErrorLine> messages) : ErrorSection("", messages) {}
    ErrorSection(std::vector<ErrorLine> messages) : ErrorSection("", messages) {}
    std::string toString(const GlobalState &gs);
};

struct ComplexError : public BasicError {
    std::vector<ErrorSection> sections;
    virtual std::string toString(const GlobalState &gs);
    ComplexError(Loc loc, ErrorClass what, std::string header, std::initializer_list<ErrorSection> sections)
        : BasicError(loc, what, header), sections(sections) {}
    ComplexError(Loc loc, ErrorClass what, std::string header, std::vector<ErrorSection> sections)
        : BasicError(loc, what, header), sections(sections) {}
    ComplexError(Loc loc, ErrorClass what, std::string header, ErrorLine other_line)
        : BasicError(loc, what, header), sections({ErrorSection({other_line})}) {}
};

/*
 * Used to batch errors in an RAII fashion:
 *
 * {
 *   core::ErrorRegion errs(gs);
 *   runNamer();
 * }
 */
class ErrorRegion {
public:
    ErrorRegion(GlobalState &gs, bool silenceErrors = false) : gs(gs), silenceErrors(silenceErrors){};
    ~ErrorRegion();

private:
    GlobalState &gs;
    bool silenceErrors;
};

} // namespace core
} // namespace ruby_typer

#endif
