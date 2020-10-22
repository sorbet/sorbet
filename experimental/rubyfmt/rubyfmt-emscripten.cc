#include "experimental/rubyfmt/rubyfmt.h"

using namespace std;

namespace experimental::rubyfmt {

RubyfmtResult format(string_view str) {
    // no-op in emscripten to avoid pulling in rubyfmt dependency.
    return {RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_OK, string(str)};
}

} // namespace experimental::rubyfmt
