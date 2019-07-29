#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerInitialize(const int *argc, const char ***argv) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const std::size_t size) {
    return 0;
}
