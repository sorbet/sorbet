#ifndef SORBET_PICKLER_H
#define SORBET_PICKLER_H

#include "common/common.h"

namespace sorbet::core::serialize {
class Pickler {
    std::vector<u1> data;
    u1 zeroCounter = 0;

public:
    void putU4(uint32_t u);
    void putU1(const u1 u);
    void putS8(const int64_t i);
    void putStr(std::string_view s);
    std::vector<u1> result();
    Pickler() = default;
};

class UnPickler {
    int pos;
    u1 zeroCounter = 0;
    std::vector<u1> data;

public:
    uint32_t getU4();
    u1 getU1();
    int64_t getS8();
    std::string_view getStr();
    explicit UnPickler(const u1 *const compressed, spdlog::logger &tracer);
};

} // namespace sorbet::core::serialize
#endif
