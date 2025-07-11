#ifndef RBS_TYPE_PARAMS_TO_PARSER_NODE_H
#define RBS_TYPE_PARAMS_TO_PARSER_NODE_H

#include "parser/parser.h"
#include "rbs/rbs_common.h"

namespace sorbet::rbs {

class TypeParamsToParserNode {
    core::MutableContext ctx;
    Parser parser;

public:
    TypeParamsToParserNode(core::MutableContext ctx, Parser parser) : ctx(ctx), parser(parser) {}

    parser::NodeVec typeParams(const rbs_node_list_t *rbsTypeParams, const RBSDeclaration &declaration);
};

} // namespace sorbet::rbs

#endif // RBS_METHOD_TYPE_TO_PARSER_NODE_H
