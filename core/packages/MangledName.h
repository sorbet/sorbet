#ifndef SORBET_CORE_PACKAGES_MANGLEDNAME_H
#define SORBET_CORE_PACKAGES_MANGLEDNAME_H

#include "core/GlobalState.h"
#include "core/LocOffsets.h"
#include "core/NameRef.h"
#include <vector>

namespace sorbet::core::packages {
class MangledName final {
public:
    static core::NameRef mangledNameFromParts(core::GlobalState &gs, std::vector<std::string_view> &parts);
    static core::NameRef mangledNameFromParts(core::GlobalState &gs, std::vector<core::NameRef> &parts);
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
} // namespace sorbet::core::packages
#endif
