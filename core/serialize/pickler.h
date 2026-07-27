#ifndef SORBET_PICKLER_H
#define SORBET_PICKLER_H

#include "absl/base/casts.h"
#include "absl/types/span.h"
#include "common/common.h"

namespace sorbet::core::serialize {
class Pickler {
    std::vector<uint8_t> data;
    uint8_t zeroCounter = 0;

public:
    void putU4(uint32_t u);
    void putU1(const uint8_t u);
    void putU8(const uint64_t i);
    void putS8(const int64_t i) {
        this->putU8(absl::bit_cast<uint64_t>(i));
    }
    void putStr(std::string_view s);
    void putBytes(absl::Span<const uint8_t> bytes);
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
    uint64_t getU8();
    int64_t getS8() {
        return absl::bit_cast<int64_t>(this->getU8());
    }
    std::string_view getStr();
    absl::Span<const uint8_t> getBytes();
    explicit UnPickler(const uint8_t *const compressed, spdlog::logger &tracer);
};

} // namespace sorbet::core::serialize
#endif
