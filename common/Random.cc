#include "common/Random.h"

using namespace std;

namespace sorbet {
random_device Random::rd;

default_random_engine Random::re(Random::rd());

uint32_t Random::uniformU4() {
    return uniformU4(0, (uint32_t)0 - (uint32_t)1);
}

uint64_t Random::uniformU8() {
    return uniformU8(0, (uint64_t)0 - (uint64_t)1);
}

uint32_t Random::uniformU4(uint32_t from, uint32_t to) {
    uniform_int_distribution<uint32_t> uniformDist(from, to);
    return uniformDist(re);
}

uint64_t Random::uniformU8(uint64_t from, uint64_t to) {
    uniform_int_distribution<uint64_t> uniformDist(from, to);
    return uniformDist(re);
}
} // namespace sorbet
