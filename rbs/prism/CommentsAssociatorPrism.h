#ifndef SORBET_RBS_COMMENTS_ASSOCIATOR_PRISM_H
#define SORBET_RBS_COMMENTS_ASSOCIATOR_PRISM_H

#include "common/common.h"
#include "parser/parser.h"
#include "parser/prism/Factory.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"
#include <memory>
#include <regex>
#include <string_view>

extern "C" {
#include "prism.h"
}

// Synthetic node markers for RBS comments
// We repurpose PM_CONSTANT_READ_NODE with special constant IDs to mark synthetic nodes
#define RBS_SYNTHETIC_BIND_MARKER PM_CONSTANT_ID_UNSET
#define RBS_SYNTHETIC_TYPE_ALIAS_MARKER (PM_CONSTANT_ID_UNSET + 1)

namespace sorbet::rbs {

struct CommentNodePrism {
    core::LocOffsets loc;
    std::string_view string;
};

struct CommentMapPrismNode {
    std::unordered_map<pm_node_t *, std::vector<CommentNodePrism>> signaturesForNode;
    std::unordered_map<pm_node_t *, std::vector<CommentNodePrism>> assertionsForNode;
};

class CommentsAssociatorPrism {
public:
    static const std::string_view RBS_PREFIX;

    CommentsAssociatorPrism(core::MutableContext ctx, parser::Prism::Parser &parser,
                            std::vector<core::LocOffsets> commentLocations);
    CommentMapPrismNode run(pm_node_t *node);

private:
    static const std::string_view ANNOTATION_PREFIX;
    static const std::string_view MULTILINE_RBS_PREFIX;
    static const std::string_view BIND_PREFIX;

    core::MutableContext ctx;
    parser::Prism::Parser &parser;
    parser::Prism::Factory prism;
    std::vector<core::LocOffsets> commentLocations;
    std::map<int, CommentNodePrism> commentByLine;
    std::unordered_map<pm_node_t *, std::vector<CommentNodePrism>> signaturesForNode;
    std::unordered_map<pm_node_t *, std::vector<CommentNodePrism>> assertionsForNode;
    std::vector<std::pair<bool, core::LocOffsets>> contextAllowingTypeAlias;
    int lastLine;

    void walkNode(pm_node_t *node);
    void walkNodes(pm_node_list_t &nodes);
    void walkArgumentsNode(pm_arguments_node_t *args);
    void walkStatements(pm_node_list_t &nodes);
    pm_node_t *walkBody(pm_node_t *node, pm_node_t *body);
    void processTrailingComments(pm_node_t *node, pm_node_list_t &nodes);
    template <typename PrismNode> void walkAssignmentNode(pm_node_t *anyNode);
    template <typename PrismNode> void walkCallAssignmentNode(pm_node_t *untypedNode, std::string_view label);
    template <typename PrismNode> void walkIndexAssignmentNode(pm_node_t *untypedNode, std::string_view label);
    void walkConditionalNode(pm_node_t *node, pm_node_t *predicate, pm_statements_node_t *&statements,
                             pm_node_t *&elsePart, std::string_view kind);
    void associateAssertionCommentsToNode(pm_node_t *node, bool adjustLocForHeredoc = false);
    void associateSignatureCommentsToNode(pm_node_t *node);
    void consumeCommentsInsideNode(pm_node_t *node, std::string_view kind);
    void consumeCommentsBetweenLines(int startLine, int endLine, std::string_view kind);
    void consumeCommentsUntilLine(int line);
    std::optional<uint32_t> locateTargetLine(pm_node_t *node);
    core::LocOffsets translateLocation(pm_location_t location);
    uint32_t posToLine(uint32_t pos);

    int maybeInsertStandalonePlaceholders(pm_node_list_t &nodes, int index, int lastLine, int currentLine);
};

} // namespace sorbet::rbs
#endif // SORBET_RBS_COMMENTS_ASSOCIATOR_PRISM_H
