#ifndef SORBET_RBS_SIGS_REWRITER_PRISM_H
#define SORBET_RBS_SIGS_REWRITER_PRISM_H

#include <memory>

#include "parser/prism/Factory.h"
#include "parser/prism/Helpers.h"
#include "parser/prism/Parser.h"
#include "rbs/prism/CommentsAssociatorPrism.h"
#include "rbs/rbs_common.h"

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
    SigsRewriterPrism(core::MutableContext ctx, parser::Prism::Parser &parser,
                      UnorderedMap<pm_node_t *, std::vector<rbs::CommentNodePrism>> &commentsByNode)
        : ctx{ctx}, parser{parser}, prism{parser}, commentsByNode{commentsByNode} {}
    pm_node_t *run(pm_node_t *node);

private:
    core::MutableContext ctx;
    parser::Prism::Parser &parser;
    parser::Prism::Factory prism;
    UnorderedMap<pm_node_t *, std::vector<rbs::CommentNodePrism>> &commentsByNode;

    pm_node_t *rewriteBody(pm_node_t *node);
    pm_statements_node_t *rewriteBody(pm_statements_node_t *stmts);
    pm_node_t *rewriteNode(pm_node_t *node);
    void rewriteNodes(pm_node_list_t &nodes);
    void rewriteArgumentsNode(pm_arguments_node_t *args);
    pm_node_t *rewriteClass(pm_node_t *node);
    std::unique_ptr<std::vector<pm_node_t *>> signaturesForNode(pm_node_t *node);
    CommentsPrism commentsForNode(pm_node_t *node);
    void insertTypeParams(pm_node_t *node, pm_node_t *body);
    void processClassBody(pm_node_t *node, pm_node_t *&body, absl::Span<pm_node_t *const> helpers);
    pm_node_t *replaceSyntheticTypeAlias(pm_node_t *node);
    pm_node_t *createStatementsWithSignatures(pm_node_t *originalNode,
                                              std::unique_ptr<std::vector<pm_node_t *>> signatures);
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_SIGS_REWRITER_PRISM_H
