#ifndef SORBET_PARSED_PARAM_H
#define SORBET_PARSED_PARAM_H

#include "core/Loc.h"
#include "core/LocalVariable.h"

namespace sorbet::core {

struct ParsedParam {
    struct Flags {
        bool isKeyword : 1 = false;
        bool isRepeated : 1 = false;
        bool isDefault : 1 = false;
        bool isShadow : 1 = false;
        bool isBlock : 1 = false;

        void setFromU1(uint8_t flags);
        uint8_t toU1() const;
    };
    core::LocOffsets loc;
    core::LocalVariable local;
    Flags flags;
};

} // namespace sorbet::core

#endif
