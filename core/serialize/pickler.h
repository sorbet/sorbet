#ifndef SORBET_PICKLER_H
#define SORBET_PICKLER_H

#include "common/common.h"

namespace sorbet::core::serialize {
class Pickler {
    std::vector<uint8_t> data;
    uint8_t zeroCounter = 0;

public:
    void putU4(uint32_t u);
    void putU1(const uint8_t u);
    void putS8(const int64_t i);
    void putStr(std::string_view s);
    std::vector<uint8_t> result();
    Pickler() = default;
};

class UnPickler {
    int pos;
    uint8_t zeroCounter = 0;
    std::vector<uint8_t> data;

public:
    uint32_t getU4();
    uint8_t getU1();
    int64_t getS8();
    std::string_view getStr();
    explicit UnPickler(const uint8_t *const compressed, spdlog::logger &tracer);
};

} // namespace sorbet::core::serialize
#endif
