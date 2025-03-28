#include "rbs/rbs_common.h"

namespace sorbet::rbs {

core::LocOffsets locFromRange(core::LocOffsets loc, const range &range) {
    return {
        loc.beginPos() + range.start.char_pos,
        loc.beginPos() + range.end.char_pos,
    };
}

} // namespace sorbet::rbs
