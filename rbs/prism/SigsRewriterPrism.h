#ifndef SORBET_RBS_SIGS_REWRITER_PRISM_H
#define SORBET_RBS_SIGS_REWRITER_PRISM_H

#include "parser/parser.h"
#include "parser/prism/Parser.h"
#include "rbs/prism/CommentsAssociatorPrism.h"
#include "rbs/rbs_common.h"
#include <memory>

extern "C" {
#include "prism.h"
}

namespace sorbet::rbs {

/**
 * A collection of annotations and signatures comments found on a method definition.
 */
struct CommentsPrism {
    /**
     * RBS annotation comments found on a method definition.
     *
     * Annotations are formatted as `@some_annotation`.
     */
    std::vector<Comment> annotations;

    /**
     * RBS signature comments found on a method definition.
     *
     * Signatures are formatted as `#: () -> void`.
     */
    std::vector<RBSDeclaration> signatures;
};

class SigsRewriterPrism {
public:
    SigsRewriterPrism(core::MutableContext ctx,
                      std::map<pm_node_t *, std::vector<rbs::CommentNodePrism>> &commentsByNode)
        : ctx(ctx), commentsByNode(&commentsByNode){};
    pm_node_t *run(pm_node_t *node);

private:
    core::MutableContext ctx;
    std::map<pm_node_t *, std::vector<rbs::CommentNodePrism>> *commentsByNode;

    pm_node_t *rewriteBegin(pm_node_t *node);
    pm_node_t *rewriteBody(pm_node_t *node);
    pm_node_t *rewriteNode(pm_node_t *node);
    void rewriteNodes(pm_node_list_t &nodes);
    pm_node_t *rewriteClass(pm_node_t *node);
    std::unique_ptr<parser::NodeVec> signaturesForNode(pm_node_t *node);
    CommentsPrism commentsForNode(pm_node_t *node);
    void insertTypeParams(parser::Node *node, std::unique_ptr<parser::Node> *body);
    std::unique_ptr<parser::Node> replaceSyntheticTypeAlias(std::unique_ptr<parser::Node> node);
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_SIGS_REWRITER_PRISM_H
