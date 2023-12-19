#ifndef SORBET_CORE_PACKAGES_MANGLEDNAME_H
#define SORBET_CORE_PACKAGES_MANGLEDNAME_H

#include "core/LocOffsets.h"
#include "core/NameRef.h"
#include <vector>

namespace sorbet::core {
class GlobalState;
} // namespace sorbet::core

namespace sorbet::core::packages {
class MangledName final {
    explicit MangledName(core::NameRef mangledName) : mangledName(mangledName) {}

public:
    core::NameRef mangledName;

    MangledName() = default;

    // ["Foo", "Bar"] => :Foo_Bar_Package
    static MangledName mangledNameFromParts(core::GlobalState &gs, const std::vector<std::string_view> &parts);
    // [:Foo, :Bar] => :Foo_Bar_Package
    static MangledName mangledNameFromParts(core::GlobalState &gs, const std::vector<core::NameRef> &parts);
    // "Foo::Bar" -> :Foo_Bar_Package
    static MangledName mangledNameFromHuman(const core::GlobalState &gs, std::string_view human);

    bool operator==(const MangledName &rhs) const {
        return mangledName == rhs.mangledName;
    }

    bool operator!=(const MangledName &rhs) const {
        return !(rhs == *this);
    }

    bool exists() const {
        return this->mangledName.exists();
    }
};

class NameFormatter final {
    const core::GlobalState &gs;

public:
    NameFormatter(const core::GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, core::NameRef name) const {
        out->append(name.shortName(gs));
    }
    void operator()(std::string *out, std::pair<core::NameRef, core::LocOffsets> p) const {
        out->append(p.first.shortName(gs));
    }
};

template <typename H> H AbslHashValue(H h, const MangledName &m) {
    return H::combine(std::move(h), m.mangledName);
}
} // namespace sorbet::core::packages
#endif
