#ifndef SORBET_REPORTER_H
#define SORBET_REPORTER_H

#include "AutocorrectSuggestion.h"
#include "Loc.h"
#include "StrictLevel.h"
#include "spdlog/fmt/fmt.h"
#include <initializer_list>
#include <memory>

namespace sorbet {
namespace core {

struct QueryResponse;

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
    std::vector<AutocorrectSuggestion> autocorrects;

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
    //  - Unreported: This error is silenced and no error will be generated or
    //    reported. No method is valid on an Unreported ErrorBuilder other than
    //    `operator bool()`.
    //
    //  - WillBuild: This error builder is live and in the process of
    //    constructing an error. This is the only state in which mutation
    //    methods (setHeader, etc) are valid.
    //
    //  - DidBuild: This error builder has built an error. No further method
    //    calls on this object are valid.
    //
    //  build() converts a WillBuild state into a DidBuild state, and is used by
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
    std::unique_ptr<BasicError> build();
};

class ErrorQueue {
private:
public:
    spdlog::logger &logger;
    spdlog::logger &tracer;
    std::atomic<bool> hadCritical{false};
    std::atomic<int> errorCount{0};

    ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer);
    virtual ~ErrorQueue();

    virtual void pushError(const GlobalState &gs, std::unique_ptr<BasicError> error) = 0;
    virtual void pushQueryResponse(std::unique_ptr<QueryResponse> error) = 0;
    virtual void flushFile(FileRef file) = 0;
    virtual void flushErrors(bool all = false) = 0;
    virtual void flushErrorCount() = 0;
    virtual void flushAutocorrects(const core::GlobalState &gs) = 0;
};

} // namespace core
} // namespace sorbet

#endif
