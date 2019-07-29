#include "test/helpers/MockFileSystem.h"
#include <cstddef>
#include <cstdint>
#include <memory>

extern "C" int LLVMFuzzerInitialize(const int *argc, const char ***argv) {
    const auto rootPath = "/tmp";
    const auto fs = std::make_shared<sorbet::test::MockFileSystem>(rootPath);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const std::size_t size) {
    return 0;
}
