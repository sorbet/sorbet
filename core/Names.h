#ifndef SORBET_NAMES_H
#define SORBET_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

#include "core/NameRef.h"
#include "core/Names_gen.h"

namespace sorbet::core {
class GlobalState;

struct UTF8Name final {
    std::string_view utf8;

    UTF8Name deepCopy(const core::GlobalState &gs) const;
};
CheckSize(UTF8Name, 16, 8);

struct UniqueName final {
    NameRef original;
    uint32_t num;
    UniqueNameKind uniqueNameKind;

    UniqueName deepCopy(const core::GlobalState &gs) const;
};

CheckSize(UniqueName, 12, 4);

struct ConstantName final {
    NameRef original;

    ConstantName deepCopy(const core::GlobalState &gs) const;
};
CheckSize(ConstantName, 4, 4);

} // namespace sorbet::core

#endif // SORBET_NAMES_H
