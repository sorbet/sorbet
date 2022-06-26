#ifndef SORBET_CFG_LOCALREF_H
#define SORBET_CFG_LOCALREF_H

#include "core/LocalVariable.h"

namespace sorbet::cfg {
class CFG;

class LocalRef final {
    uint32_t _id;

public:
    LocalRef() : _id(0){};
    LocalRef(uint32_t id) : _id(id){};
    LocalRef(const LocalRef &) = default;
    LocalRef(LocalRef &&) = default;
    LocalRef &operator=(LocalRef &&) = default;
    LocalRef &operator=(const LocalRef &) = default;

    core::LocalVariable data(const CFG &cfg) const;
    uint32_t id() const {
        return this->_id;
    }
    int minLoops(const CFG &cfg) const;
    int maxLoopWrite(const CFG &cfg) const;
    bool exists() const;
    bool isAliasForGlobal(const core::GlobalState &gs, const CFG &cfg) const;
    bool isSyntheticTemporary(const CFG &cfg) const;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg) const;
    bool operator==(const LocalRef &rhs) const;
    bool operator!=(const LocalRef &rhs) const;

    static LocalRef noVariable();
    static LocalRef blockCall();
    static LocalRef selfVariable();
    static LocalRef unconditional();
    static LocalRef finalReturn();
};
CheckSize(LocalRef, 4, 4);

template <typename H> H AbslHashValue(H h, const LocalRef &m) {
    return H::combine(std::move(h), m.id());
}
} // namespace sorbet::cfg

#endif
