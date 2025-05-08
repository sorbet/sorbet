#ifndef SORBET_CORE_PACKAGES_MANGLEDNAME_H
#define SORBET_CORE_PACKAGES_MANGLEDNAME_H

#include "core/LocOffsets.h"
#include "core/NameRef.h"
#include "core/SymbolRef.h"
#include <vector>

namespace sorbet::core {
class GlobalState;
} // namespace sorbet::core

namespace sorbet::core::packages {
class MangledName final {
    MangledName(ClassOrModuleRef owner) : owner(owner) {}

    template <typename H> friend H AbslHashValue(H h, const MangledName &m);

public:
    // The ClassOrModuleRef that this package is stored in.
    //
    // This is only to ease the transition to storing packages in the symbol table--eventually,
    // there will just be a symbol kind for packages.
    //
    // I've called this owner because when Packages are in the symbol table, they will be owned
    // by ClassOrModule symbols.
    ClassOrModuleRef owner;

    MangledName() = default;

    // ["Foo", "Bar"] => :Foo_Bar
    static MangledName mangledNameFromParts(GlobalState &gs, const std::vector<std::string_view> &parts,
                                            ClassOrModuleRef owner);
    // [:Foo, :Bar] => :Foo_Bar
    static MangledName mangledNameFromParts(GlobalState &gs, const std::vector<NameRef> &parts, ClassOrModuleRef owner);

    // [:Foo, :Bar] => :Foo_Bar
    // (might not exist)
    static MangledName lookupMangledName(const core::GlobalState &gs, const std::vector<std::string> &parts);

    bool operator==(const MangledName &rhs) const {
        return owner == rhs.owner;
    }

    bool operator!=(const MangledName &rhs) const {
        return !(rhs == *this);
    }

    bool exists() const {
        return this->owner.exists();
    }
};

class NameFormatter final {
    const GlobalState &gs;

public:
    NameFormatter(const GlobalState &gs) : gs(gs) {}

    void operator()(std::string *out, NameRef name) const {
        out->append(name.shortName(gs));
    }
    void operator()(std::string *out, std::pair<NameRef, LocOffsets> p) const {
        out->append(p.first.shortName(gs));
    }
};

template <typename H> H AbslHashValue(H h, const MangledName &m) {
    return H::combine(std::move(h), m.owner);
}
} // namespace sorbet::core::packages
#endif
