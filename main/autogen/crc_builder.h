#ifndef AUTOGEN_CRC_H
#define AUTOGEN_CRC_H

#include "common/common.h"

namespace sorbet::autogen {

/**
 * Efficiently calculates CRCs using a lookup table.
 */
class CRCBuilder {
public:
    static std::shared_ptr<CRCBuilder> create();

    CRCBuilder() = default;
    virtual ~CRCBuilder() = 0;
    // No need to copy; CRCBuilder can be shared between threads.
    CRCBuilder(CRCBuilder &) = delete;
    CRCBuilder(const CRCBuilder &) = delete;
    CRCBuilder &operator=(CRCBuilder &&) = delete;
    CRCBuilder &operator=(const CRCBuilder &) = delete;

    virtual sorbet::u4 crc32(std::string_view data) const = 0;
};

} // namespace sorbet::autogen
#endif // AUTOGEN_CRC_H
