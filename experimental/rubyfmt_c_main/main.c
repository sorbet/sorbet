#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rubyfmt.h"

int main() {
    unsigned char* ruby_source = (unsigned char*)"puts 'hi'\n";
    int res = rubyfmt_init();
    if (res != RUBYFMT_INIT_STATUS_OK) {
        fprintf(stderr, "failed to init\n");
        exit(1);
    }
    enum Rubyfmt_FormatError status = RUBYFMT_FORMAT_ERROR_OK;
    RubyfmtString* out = rubyfmt_format_buffer(ruby_source, strlen((const char*)ruby_source)-1, &status);
    if (status != 0) {
        exit(status);
    }

    unsigned char* bytes = rubyfmt_string_ptr(out);
    size_t len = rubyfmt_string_len(out);
    fwrite(bytes, sizeof(char), len, stdout);
    fflush(stdout);
    rubyfmt_string_free(out);
}
