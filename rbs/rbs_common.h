#ifndef SORBET_RBS_COMMON_H
#define SORBET_RBS_COMMON_H

extern "C" {
#include "include/rbs.h"
}

#include "core/LocOffsets.h"
#include "rbs/Parser.h"

namespace sorbet::rbs {

/**
 * A single RBS type comment found on a method definition or accessor.
 *
 * RBS type comments are formatted as `#: () -> void` for methods or `#: Integer` for attributes.
 */
struct Comment {
    core::LocOffsets commentLoc; // The location of the comment in the file
    core::LocOffsets typeLoc;    // The location of the actual RBS content in the file
    std::string_view string;     // The type string (excluding the `#: ` prefix)
                                 // this is only a view on the string owned by the File.source() data.
};

/**
 * A collection of RBS type comments that collectively describe a method signature, attribute type, or type assertion.
 */
class RBSDeclaration {
public:
    /**
     * All the comments into a single string.
     */
    std::string string;

    RBSDeclaration(std::vector<Comment> comments) : comments(std::move(comments)) {
        std::string result;
        for (const auto &comment : this->comments) {
            result += comment.string;
        }
        this->string = std::move(result);
    }

    /**
     * Returns the location that all the comments cover.
     *
     * For multiline comments, this starts at the first comment's location and ends at the last comment's location.
     *
     * ```
     * v starts from here
     * #: (Integer) ->
     * #| void
     *       ^ ends here
     * ```
     */
    core::LocOffsets commentLoc() const;

    /**
     * Returns entire type location
     *
     * ```
     *   v starts from here
     * #: (Integer) ->
     * #| void
     *       ^ ends here
     * ```
     */
    core::LocOffsets fullTypeLoc() const;

    /**
     * Returns the type location of the first comment.
     *
     * ```
     *   v starts from here
     * #: (Integer) ->
     *               ^ ends here
     * #| void
     * ```
     *
     * Usually used for generating `sig` nodes only.
     */
    core::LocOffsets firstLineTypeLoc() const;

    /**
     * Returns the type location of the RBS declaration that covers the given range.
     * For example, if given the range of the `void` token:
     *
     * ```
     * #: (Integer) ->
     *    v starts from here
     * #| void
     *       ^ ends here
     * ```
     */
    core::LocOffsets typeLocFromRange(const rbs_range_t &range) const;

private:
    std::vector<Comment> comments;
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_COMMON_H
