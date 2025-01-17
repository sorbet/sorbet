#ifndef RBS_PARSER_H
#define RBS_PARSER_H

#include "core/Context.h"
#include "rbs_common.h"

namespace sorbet::rbs {

class RBSParser {
public:
    /**
     * Parse an RBS method signature comment string into a RBS AST node.
     *
     * This is used to parse comments like `#: () -> void` found on method definitions.
     */
    static std::optional<MethodType> parseSignature(core::Context ctx, Comment comment);

    /**
     * Parse an RBS type string into a RBS AST node.
     *
     * This is used to parse comments like `#: Integer` found on attribute accessors.
     */
    static std::unique_ptr<Type> parseType(core::Context ctx, Comment comment);
};

} // namespace sorbet::rbs

#endif // RBS_PARSER_H
