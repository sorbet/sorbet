#ifndef SORBET_REPORTER_H
#define SORBET_REPORTER_H

#include "core/AutocorrectSuggestion.h"
#include "core/Loc.h"
#include "core/StrictLevel.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>
#include <memory>

namespace sorbet::core {

class ErrorClass {
public:
    const u2 code;
    const StrictLevel minLevel;

    constexpr ErrorClass(u2 code, StrictLevel minLevel) : code(code), minLevel(minLevel){};
    ErrorClass(const ErrorClass &rhs) = default;

    bool operator==(const ErrorClass &rhs) const {
        return code == rhs.code;
    }

    bool operator!=(const ErrorClass &rhs) const {
        return !(*this == rhs);
    }
};

class ErrorColors {
    static constexpr std::string_view coloredPatternSigil = "`{}`";
    static std::string coloredPatternReplace;
    static std::string replaceAll(std::string_view inWhat, std::string_view from, std::string_view to);

public:
    ErrorColors() = delete;
    template <typename... Args> static std::string format(std::string_view msg, const Args &... args) {
        return fmt::format(replaceAll(msg, coloredPatternSigil, coloredPatternReplace), args...);
    }
    static void enableColors();
    static void disableColors();
};

struct ErrorLine {
    const Loc loc;
    const std::string formattedMessage;
    ErrorLine(Loc loc, std::string formattedMessage) : loc(loc), formattedMessage(move(formattedMessage)){};

    template <typename... Args> static ErrorLine from(Loc loc, std::string_view msg, const Args &... args) {
        std::string formatted = ErrorColors::format(msg, args...);
        return ErrorLine(loc, move(formatted));
    }
    std::string toString(const GlobalState &gs, bool color = true) const;
};

struct ErrorSection {
    std::string header;
    std::vector<ErrorLine> messages;
    ErrorSection(std::string_view header) : header(header) {}
    ErrorSection(std::string_view header, const std::initializer_list<ErrorLine> &messages)
        : header(header), messages(messages) {}
    ErrorSection(std::string_view header, const std::vector<ErrorLine> &messages)
        : header(header), messages(messages) {}
    ErrorSection(const std::initializer_list<ErrorLine> &messages) : ErrorSection("", messages) {}
    ErrorSection(const std::vector<ErrorLine> &messages) : ErrorSection("", messages) {}
    std::string toString(const GlobalState &gs) const;
};

class Error {
public:
    const Loc loc;
    const ErrorClass what;
    const std::string header;
    const bool isSilenced;
    std::vector<AutocorrectSuggestion> autocorrects;
    const std::vector<ErrorSection> sections;

    bool isCritical() const;
    std::string toString(const GlobalState &gs) const;
    Error(Loc loc, ErrorClass what, std::string header, std::vector<ErrorSection> sections,
          std::vector<AutocorrectSuggestion> autocorrects, bool isSilenced)
        : loc(loc), what(what), header(move(header)), isSilenced(isSilenced), autocorrects(move(autocorrects)),
          sections(sections) {
        ENFORCE(this->header.empty() || this->header.back() != '.');
        ENFORCE(this->header.find('\n') == std::string::npos, "{} has a newline in it", this->header);
    }
};

/*
 * Used to batch errors in an RAII fashion:
 *
 * {
 *   ErrorRegion errs(gs);
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

class ErrorBuilder {
    // An ErrorBuilder can be in three states:
    //
    //  - Unreported: This error is silenced, only basic information about the error will be generated.
    //    No method is valid on an Unreported ErrorBuilder other than
    //    `operator bool()` and `build()`
    //
    //  - WillBuild: This error builder is live and in the process of
    //    constructing an error. This is the only state in which mutation
    //    methods (setHeader, etc) are valid.
    //
    //  - DidBuild: This error builder has built an error. No further method
    //    calls on this object are valid.
    //
    //  build() converts into a DidBuild state, and is used by
    //  callers who need finer-grained control over error reporting than the
    //  default behavior of reporting on destruction.
    enum class State {
        Unreported,
        WillBuild,
        DidBuild,
    };
    const GlobalState &gs;
    State state;
    Loc loc;
    ErrorClass what;
    std::string header;
    std::vector<ErrorSection> sections;
    std::vector<AutocorrectSuggestion> autocorrects;
    void _setHeader(std::string &&header);

public:
    ErrorBuilder(const ErrorBuilder &) = delete;
    ErrorBuilder(ErrorBuilder &&) = default;
    ErrorBuilder(const GlobalState &gs, bool willBuild, Loc loc, ErrorClass what);
    ~ErrorBuilder();

    inline explicit operator bool() const {
        ENFORCE(state != State::DidBuild);
        return state == State::WillBuild;
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

    void addAutocorrect(AutocorrectSuggestion &&autocorrect);
    template <typename... Args> void replaceWith(Loc loc, const std::string &msg, const Args &... args) {
        std::string formatted = fmt::format(msg, args...);
        addAutocorrect(AutocorrectSuggestion(loc, move(formatted)));
    }

    // build() builds and returns the reported Error. Only valid if state ==
    // WillBuild. This passes ownership of the error to the caller; ErrorBuilder
    // will no longer report the error, and is the caller's responsibility to
    // pass it to GlobalState::_error if the error should actually be recorded.
    std::unique_ptr<Error> build();
};

template <typename H> H AbslHashValue(H h, const ErrorClass &m) {
    return H::combine(std::move(h), m.code);
}
} // namespace sorbet::core

#endif
