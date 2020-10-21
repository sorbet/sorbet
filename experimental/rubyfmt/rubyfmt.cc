#include "experimental/rubyfmt/rubyfmt.h"

using namespace std;

extern "C" {
typedef struct _RubyfmtString RubyfmtString;

int RUBYFMT_INIT_STATUS_UNINITIALIZED = -1;
int RUBYFMT_INIT_STATUS_OK = 0;
int RUBYFMT_INIT_STATUS_ERROR = 1;

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
}

namespace experimental::rubyfmt {

static int initStatus = RUBYFMT_INIT_STATUS_UNINITIALIZED;

RubyfmtResult format(string_view str) {
    // Lazily initialize Rubyfmt.
    // Note: As stated in header, this isn't thread safe.
    if (initStatus == RUBYFMT_INIT_STATUS_UNINITIALIZED) {
        initStatus = rubyfmt_init();
    }

    // TODO: Copy not needed -- rubyfmt doesn't dirty buffer -- but right now
    // the API requests a non-const pointer.
    string copy(str);
    unsigned char *buf = reinterpret_cast<unsigned char *>(copy.data());

    RubyfmtResult result;

    RubyfmtString *out = rubyfmt_format_buffer(buf, copy.length(), &result.status);
    if (result.status == Rubyfmt_FormatError::RUBYFMT_FORMAT_ERROR_OK) {
        unsigned char *bytes = rubyfmt_string_ptr(out);
        size_t len = rubyfmt_string_len(out);
        result.formatted.assign(reinterpret_cast<char *>(bytes), len);
    }
    rubyfmt_string_free(out);
    return result;
}

} // namespace experimental::rubyfmt
