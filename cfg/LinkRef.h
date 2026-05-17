#ifndef SORBET_CFG_LINKREF_H
#define SORBET_CFG_LINKREF_H

#include "common/common.h"

namespace sorbet::cfg {

class LinkRef final {
    uint32_t _id = 0;

public:
    LinkRef() = default;
    explicit LinkRef(uint32_t id) noexcept : _id(id) {}
    LinkRef(const LinkRef &) = default;
    LinkRef(LinkRef &&) = default;
    LinkRef &operator=(LinkRef &&) = default;
    LinkRef &operator=(const LinkRef &) = default;

    bool exists() const noexcept {
        return _id != 0;
    }

    uint32_t id() const noexcept {
        return this->_id;
    }
};
CheckSize(LinkRef, 4, 4);

} // namespace sorbet::cfg

#endif
