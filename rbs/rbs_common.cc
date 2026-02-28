#include "rbs/rbs_common.h"

namespace sorbet::rbs {

rbs_string_t makeRBSString(std::string_view str) {
    return rbs_string_new(str.data(), str.data() + str.size());
}

core::LocOffsets RBSDeclaration::commentLoc() const {
    return comments.front().commentLoc.join(comments.back().commentLoc);
}

core::LocOffsets RBSDeclaration::firstLineTypeLoc() const {
    return comments.front().typeLoc;
}

core::LocOffsets RBSDeclaration::fullTypeLoc() const {
    return comments.front().typeLoc.join(comments.back().typeLoc);
}

core::LocOffsets RBSDeclaration::typeLocFromRange(const rbs_location_range &range) const {
    int rangeOffset = range.start_char;
    int rangeLength = range.end_char - range.start_char;

    for (const auto &comment : comments) {
        int commentTypeLength = comment.typeLoc.endLoc - comment.typeLoc.beginLoc;
        if (rangeOffset < commentTypeLength) {
            auto beginLoc = comment.typeLoc.beginLoc + rangeOffset;
            auto endLoc = beginLoc + rangeLength;

            if (rangeLength > commentTypeLength) {
                endLoc = comment.typeLoc.endLoc;
            }

            return core::LocOffsets{beginLoc, endLoc};
        }
        rangeOffset -= commentTypeLength;
    }
    return comments.front().typeLoc;
}

} // namespace sorbet::rbs
