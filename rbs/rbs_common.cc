#include "rbs/rbs_common.h"

namespace sorbet::rbs {

core::LocOffsets locFromRange(core::LocOffsets loc, const range &range) {
    return {
        loc.beginPos() + range.start.char_pos + 2,
        loc.beginPos() + range.end.char_pos + 2,
    };
}

} // namespace sorbet::rbs
