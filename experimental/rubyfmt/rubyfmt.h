#ifndef EXPERIMENTAL_RUBYFMT_H
#define EXPERIMENTAL_RUBYFMT_H

#include <string>
#include <string_view>

namespace experimental::rubyfmt {

enum RubyfmtFormatError {
    RUBYFMT_FORMAT_ERROR_OK = 0,

    // passed buffer contained a ruby syntax error. Non fatal, user should feel
    // free to continue to call rubyfmt with non-error strings.
    RUBYFMT_FORMAT_ERROR_SYNTAX_ERROR = 1,

    // this error is fatal, the calling program should not continue to execute
    // rubyfmt and you should report a bug with the file that crashed rubyfmt
    RUBYFMT_FORMAT_ERROR_RIPPER_PARSE_FAILURE = 2,

    // an error occured during IO within the function, should be impossible
    // and most likely indicates a programming error within rubyfmt, please
    // file a bug
    RUBYFMT_FORMAT_ERROR_IO_ERROR = 3,

    // some unknown ruby error occured during execution fo Rubyfmt. This indicates
    // a programming error. Please file a bug report and terminate the process
    // and restart.
    RUBYFMT_OTHER_RUBY_ERROR = 4,

    // Rubyformat failed to initialize.
    RUBYFMT_INITIALIZE_ERROR = 9999,
};

struct RubyfmtResult {
    RubyfmtFormatError status;
    std::string formatted;
};

// Formats the given string through RubyFmt.
RubyfmtResult format(std::string_view str);
} // namespace experimental::rubyfmt

#endif
