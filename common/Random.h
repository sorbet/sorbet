#ifndef SORBET_RANDOM_H
#define SORBET_RANDOM_H

#include "common/common.h"
#include <random>

namespace sorbet {
class Random {
private:
    static std::random_device rd;
    static std::default_random_engine re;

public:
    static u4 uniformU4();
    static u4 uniformU4(u4 from, u4 to);
    static uint64_t uniformU8();
    static uint64_t uniformU8(uint64_t from, uint64_t to);
};
} // namespace sorbet

#endif // SORBET_RANDOM_H
