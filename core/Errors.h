#ifndef SORBET_REPORTER_H
#define SORBET_REPORTER_H

#include "Loc.h"
#include "StrictLevel.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>
#include <memory>

namespace ruby_typer {
namespace core {

class ErrorClass {
public:
    u2 code;
    StrictLevel minLevel;

    constexpr ErrorClass(u2 code, StrictLevel minLevel) : code(code), minLevel(minLevel){};
    ErrorClass(const ErrorClass &rhs) = default;
    ErrorClass &operator=(const ErrorClass &rhs) = default;

    bool operator==(const ErrorClass &rhs) const {
        return code == rhs.code;
    }

    bool operator!=(const ErrorClass &rhs) const {
        return !(*this == rhs);
    }
};

class ErrorColors {
    static const std::string coloredPatternSigil;
    static std::string coloredPatternReplace;
    static std::string replaceAll(const std::string &inWhat, const std::string &from, const std::string &to);

public:
    ErrorColors() = delete;
    template <typename... Args> static std::string format(const std::string &msg, const Args &... args) {
        return fmt::format(replaceAll(msg, coloredPatternSigil, coloredPatternReplace), args...);
    }
    static void enableColors();
    static void disableColors();
};

struct BasicError {
    Loc loc;
    ErrorClass what;
    std::string formatted;
    bool isCritical;

    BasicError(Loc loc, ErrorClass what, std::string formatted);

    virtual std::string toString(const GlobalState &gs);
    virtual ~BasicError() = default;
};

struct ErrorLine {
    Loc loc;
    std::string formattedMessage;
    ErrorLine(Loc loc, std::string formattedMessage) : loc(loc), formattedMessage(formattedMessage){};

    template <typename... Args> static ErrorLine from(Loc loc, const std::string &msg, const Args &... args) {
        std::string formatted = ErrorColors::format(msg, args...);
        return ErrorLine(loc, formatted);
    }
    std::string toString(const GlobalState &gs, bool color = true);
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
    ErrorRegion(const GlobalState &gs, FileRef f) : gs(gs), f(f){};
    ~ErrorRegion();

private:
    const GlobalState &gs;
    FileRef f;
};

struct ErrorQueueMessage {
    enum class Kind { Error, Flush, Drop };
    Kind kind;
    FileRef whatFile;
    std::string text;
    std::unique_ptr<BasicError> error;
};

class ErrorBuilder {
    const GlobalState &gs;
    bool willBuild;
    Loc loc;
    ErrorClass what;
    std::string header;
    std::vector<ErrorSection> sections;
    void _setHeader(std::string &&header);

public:
    ErrorBuilder(const ErrorBuilder &) = delete;
    ErrorBuilder(ErrorBuilder &&) = default;
    ErrorBuilder(const GlobalState &gs, bool willBuild, Loc loc, ErrorClass what);
    ~ErrorBuilder();
    inline explicit operator bool() const {
        return willBuild;
    }
    void addErrorSection(ErrorSection &&section);
    template <typename... Args> void addErrorLine(Loc loc, const std::string &msg, const Args &... args) {
        std::string formatted = ErrorColors::format(msg, args...);
        addErrorSection(ErrorSection({ErrorLine(loc, formatted)}));
    }

    template <typename... Args> void setHeader(const std::string &msg, const Args &... args) {
        std::string formatted = ErrorColors::format(msg, args...);
        _setHeader(move(formatted));
    }
};

} // namespace core
} // namespace ruby_typer

#endif
