#ifndef RUBYFMT_H
#define RUBYFMT_H

int RUBYFMT_INIT_STATUS_OK = 0;
int RUBYFMT_INIT_STATUS_ERROR = 1;

enum Rubyfmt_FormatError {
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
};

typedef struct _RubyfmtString RubyfmtString;

// setup rubyfmt, call once per process. Will return non zero (RUBYFMT_INIT_STATUS_ERROR)
// if initialization failed
int rubyfmt_init();

// ask rubyfmt to format the passed buffer. Must be utf-8 encoded, and len
// bytes long. Returns NULL and populates the err pointer with non zero if
// an error occurs
RubyfmtString *rubyfmt_format_buffer(unsigned char *buf, size_t len, enum Rubyfmt_FormatError *err);

// free a RubyfmtString after use
void rubyfmt_string_free(RubyfmtString *);

// Get a byte pointer from a RubyfmtString, is not a null terminated string,
// but instead should be used with rubyfmt_string_len to get the number
// of bytes in the string
unsigned char *rubyfmt_string_ptr(const RubyfmtString *);
size_t rubyfmt_string_len(const RubyfmtString *);

#endif
