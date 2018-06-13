#ifndef SORBET_RANDOM_H
#define SORBET_RANDOM_H

#include "common.h"
#include <random>

namespace sorbet {
class Random {
private:
    static std::random_device rd;
    static std::default_random_engine re;

public:
    static u4 uniformU4();
    static u4 uniformU4(u4 from, u4 to);
    static u8 uniformU8();
    static u8 uniformU8(u8 from, u8 to);
};
} // namespace sorbet

#endif // SORBET_RANDOM_H
