#ifndef SORBET_PARSER_PRISM_TRANSLATOR_H
#define SORBET_PARSER_PRISM_TRANSLATOR_H

#include "../Node.h" // To clarify: these are Sorbet Parser nodes, not Prism ones.
#include "core/errors/parser.h"
#include "parser/prism/Parser.h"
#include <memory>

extern "C" {
#include "prism.h"
}

namespace sorbet::parser::Prism {

class Translator final {
    Parser parser;

    // The functions in Pipeline.cc pass around a reference to the global state as a parameter,
    // but don't have explicit ownership over it. We take a temporary reference to it, but we can't
    // escape that scope, which is why Translator objects can't be copied, or even moved.
    core::GlobalState &gs;

    // Needed for reporting diagnostics
    core::FileRef file;

    // Context variables
    bool isInMethodDef = false;

    // Keep track of the unique ID counter
    // uniqueCounterStorage is the source of truth maintained by the parent Translator
    // uniqueCounter is a pointer to uniqueCounterStorage and is passed down to child Translators
    int uniqueCounterStorage;
    int *uniqueCounter;

    Translator(Translator &&) = delete;                 // Move constructor
    Translator(const Translator &) = delete;            // Copy constructor
    Translator &operator=(Translator &&) = delete;      // Move assignment
    Translator &operator=(const Translator &) = delete; // Copy assignment
public:
    Translator(Parser parser, core::GlobalState &gs, core::FileRef file)
        : parser(std::move(parser)), gs(gs), file(file), uniqueCounterStorage(1),
          uniqueCounter(&this->uniqueCounterStorage) {}

    int nextUniqueID() {
        return *uniqueCounter += 1;
    }

    // Translates the given AST from Prism's node types into the equivalent AST in Sorbet's legacy parser node types.
    std::unique_ptr<parser::Node> translate(pm_node_t *node);
    std::unique_ptr<parser::Node> translate(const Node &node);

private:
    // Private constructor used only for creating child translators
    // uniqueCounterStorage is passed as the minimum integer value and is never used
    Translator(Parser parser, core::GlobalState &gs, core::FileRef file, bool isInMethodDef, int *uniqueCounter)
        : parser(parser), gs(gs), file(file), isInMethodDef(isInMethodDef),
          uniqueCounterStorage(std::numeric_limits<int>::min()), uniqueCounter(uniqueCounter) {}
    void reportError(core::LocOffsets loc, const std::string &message);

    core::LocOffsets translateLoc(pm_location_t loc);

    parser::NodeVec translateMulti(pm_node_list prismNodes);
    void translateMultiInto(NodeVec &sorbetNodes, absl::Span<pm_node_t *> prismNodes);

    NodeVec translateArguments(pm_arguments_node *node, pm_node *blockArgumentNode = nullptr);
    std::unique_ptr<parser::Hash> translateHash(pm_node_t *node, pm_node_list_t elements,
                                                bool isUsedForKeywordArguments);
    std::unique_ptr<parser::Node> translateCallWithBlock(pm_node_t *prismBlockOrLambdaNode,
                                                         std::unique_ptr<parser::Node> sendNode);
    std::unique_ptr<parser::Node> translateRescue(pm_rescue_node *prismRescueNode,
                                                  std::unique_ptr<parser::Node> beginNode,
                                                  std::unique_ptr<parser::Node> elseNode);
    std::unique_ptr<parser::Node> translateStatements(pm_statements_node *stmtsNode, bool inlineIfSingle);

    template <typename SorbetNode> std::unique_ptr<SorbetNode> translateSimpleKeyword(pm_node_t *untypedNode);

    std::unique_ptr<parser::Regopt> translateRegexpOptions(pm_location_t closingLoc);
    std::unique_ptr<parser::Regexp> translateRegexp(pm_string_t unescaped, core::LocOffsets location,
                                                    pm_location_t closingLoc);

    template <typename PrismNode> std::unique_ptr<parser::Mlhs> translateMultiTargetLhs(PrismNode *);

    template <typename PrismAssignmentNode, typename SorbetLHSNode>
    std::unique_ptr<parser::Assign> translateAssignment(pm_node_t *node);

    template <typename PrismAssignmentNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
    std::unique_ptr<SorbetAssignmentNode> translateOpAssignment(pm_node_t *node);

    template <typename PrismLhsNode, typename SorbetLHSNode>
    std::unique_ptr<parser::Node> translateConst(PrismLhsNode *node, bool skipDynamicConstantWorkaround = false);

    // Pattern-matching
    // ... variations of the main translation functions for pattern-matching related nodes.
    std::unique_ptr<parser::Node> patternTranslate(pm_node_t *node);
    parser::NodeVec patternTranslateMulti(pm_node_list prismNodes);
    void patternTranslateMultiInto(NodeVec &sorbetNodes, absl::Span<pm_node_t *> prismNodes);

    // Context management helpers. These return a copy of `this` with some change to the context.
    Translator enterMethodDef();
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_TRANSLATOR_H
