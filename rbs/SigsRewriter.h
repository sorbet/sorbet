#ifndef SORBET_RBS_SIGS_REWRITER_H
#define SORBET_RBS_SIGS_REWRITER_H

#include "parser/parser.h"
#include "rbs/rbs_common.h"
#include <memory>

namespace sorbet::rbs {

class SigsRewriter {
public:
    /**
     * A collection of annotations and signatures comments found on a method definition.
     */
    struct Comments {
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
        std::vector<Comment> signatures;
    };

    SigsRewriter(core::MutableContext ctx) : ctx(ctx){};
    std::unique_ptr<parser::Node> run(std::unique_ptr<parser::Node> tree);

private:
    core::MutableContext ctx;
    UnorderedMap<uint32_t, Comments> methodSignatures;

    std::unique_ptr<parser::Node> rewriteBegin(std::unique_ptr<parser::Node> tree);
    std::unique_ptr<parser::Node> rewriteBody(std::unique_ptr<parser::Node> tree);
    std::unique_ptr<parser::Node> rewriteNode(std::unique_ptr<parser::Node> tree);
    parser::NodeVec rewriteNodes(parser::NodeVec nodes);
    void extractCommentsFromFile();
    void checkForUnusedComments();
};

} // namespace sorbet::rbs

#endif // SORBET_RBS_SIGS_REWRITER_H
