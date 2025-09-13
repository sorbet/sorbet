#ifndef SORBET_PARSED_PARAM_H
#define SORBET_PARSED_PARAM_H

#include "core/Loc.h"
#include "core/LocalVariable.h"

namespace sorbet::core {

struct ParsedParam {
    struct ArgFlags {
        bool isKeyword : 1;
        bool isRepeated : 1;
        bool isDefault : 1;
        bool isShadow : 1;
        bool isBlock : 1;

        // In C++20 we can replace this with bit field initialzers
        ArgFlags() : isKeyword(false), isRepeated(false), isDefault(false), isShadow(false), isBlock(false) {}

        void setFromU1(uint8_t flags);
        uint8_t toU1() const;
    };
    core::LocOffsets loc;
    core::LocalVariable local;
    ArgFlags flags;
};

} // namespace sorbet::core

#endif
