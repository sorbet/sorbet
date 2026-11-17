#ifndef SORBET_CFG_LOCALREF_H
#define SORBET_CFG_LOCALREF_H

#include "common/common.h"
#include "core/LocalVariable.h"

namespace sorbet::cfg {
class CFG;

// For a local variable, the outermost loop depth at which it's read or written, and whether its type is
// pinned for the whole method (instead of only across loop iterations).
struct MinLoops {
    enum class Pin : uint32_t {
        None = 0,
        Field = 1,
        Let = 2,
    };

    // If we have a file with more than 500M nesting levels, we have bigger problems.
    static constexpr int32_t MAX_LEVEL = (1 << 29) - 1;

    int32_t level : 30 = MAX_LEVEL;
    Pin pin : 2 = Pin::None;
};
CheckSize(MinLoops, 4, 4);

class LocalRef final {
    uint32_t _id;

public:
    LocalRef() : _id(0) {};
    LocalRef(uint32_t id) : _id(id) {};
    LocalRef(const LocalRef &) = default;
    LocalRef(LocalRef &&) = default;
    LocalRef &operator=(LocalRef &&) = default;
    LocalRef &operator=(const LocalRef &) = default;

    core::LocalVariable data(const CFG &cfg) const;
    uint32_t id() const {
        return this->_id;
    }
    MinLoops minLoops(const CFG &cfg) const;
    int maxLoopWrite(const CFG &cfg) const;
    bool exists() const;
    bool isAliasForGlobal(const core::GlobalState &gs, const CFG &cfg) const;
    bool isSyntheticTemporary(const CFG &cfg) const;
    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg) const;

    bool operator==(const LocalRef &rhs) const noexcept = default;
    bool operator!=(const LocalRef &rhs) const noexcept = default;

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
