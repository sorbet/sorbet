#ifndef RBS_METHOD_TYPE_TO_PARSER_NODE_H
#define RBS_METHOD_TYPE_TO_PARSER_NODE_H

#include "parser/parser.h"
#include "rbs/rbs_common.h"

namespace sorbet::rbs {

class MethodTypeToParserNode {
    core::MutableContext ctx;
    Parser parser;

public:
    MethodTypeToParserNode(core::MutableContext ctx, Parser parser) : ctx(ctx), parser(parser) {}

    /**
     * Convert an RBS method signature comment to a Sorbet signature.
     *
     * For example the signature comment `#: () -> void` will be translated as `sig { void }`.
     */
    std::unique_ptr<parser::Node> methodSignature(const parser::Node *methodDef, const rbs_methodtype_t *methodType,
                                                  const core::LocOffsets typeLoc, const core::LocOffsets commentLoc,
                                                  const std::vector<Comment> &annotations);

    /**
     * Convert an RBS attribute type comment to a Sorbet signature.
     *
     * For example the attribute type comment `#: Integer` will be translated as `sig { returns(Integer) }`.
     */
    std::unique_ptr<parser::Node> attrSignature(const parser::Send *send, const rbs_node_t *type,
                                                const core::LocOffsets typeLoc, const core::LocOffsets commentLoc,
                                                const std::vector<Comment> &annotations);
};

} // namespace sorbet::rbs

#endif // RBS_METHOD_TYPE_TO_PARSER_NODE_H
