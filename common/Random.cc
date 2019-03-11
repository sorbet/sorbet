#include "common/Random.h"

using namespace std;

namespace sorbet {
random_device Random::rd;

default_random_engine Random::re(Random::rd());

u4 Random::uniformU4() {
    return uniformU4(0, (u4)0 - (u4)1);
}

u8 Random::uniformU8() {
    return uniformU8(0, (u8)0 - (u8)1);
}

u4 Random::uniformU4(u4 from, u4 to) {
    uniform_int_distribution<u4> uniformDist(from, to);
    return uniformDist(re);
}

u8 Random::uniformU8(u8 from, u8 to) {
    uniform_int_distribution<u8> uniformDist(from, to);
    return uniformDist(re);
}
} // namespace sorbet
