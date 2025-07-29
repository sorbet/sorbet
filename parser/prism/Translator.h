#ifndef SORBET_PARSER_PRISM_TRANSLATOR_H
#define SORBET_PARSER_PRISM_TRANSLATOR_H

#include "absl/types/span.h"
#include "core/errors/parser.h"
#include "parser/Node.h" // To clarify: these are Sorbet Parser nodes, not Prism ones.
#include "parser/prism/Parser.h"
#include <memory>

extern "C" {
#include "prism.h"
}

namespace sorbet::parser::Prism {

class Translator final {
    const Parser &parser;
    // This context holds a reference to the GlobalState allocated up the call stack, which is why we don't allow
    // Translator objects to be copied or moved.
    core::MutableContext ctx;

    // The errors that were found by Prism during parsing
    const absl::Span<const ParseError> parseErrors;

    // Whether to directly desugar during in the Translator, or wait until the usual `Desugar.cc` code path.
    bool directlyDesugar;

    // Keep track of the unique ID counter
    // uniqueCounterStorage is the source of truth maintained by the parent Translator
    // uniqueCounter is a pointer to uniqueCounterStorage and is passed down to child Translators
    int uniqueCounterStorage;
    int *uniqueCounter;

    // Context variables
    bool isInMethodDef = false;

    Translator(Translator &&) = delete;                 // Move constructor
    Translator(const Translator &) = delete;            // Copy constructor
    Translator &operator=(Translator &&) = delete;      // Move assignment
    Translator &operator=(const Translator &) = delete; // Copy assignment
public:
    Translator(const Parser &parser, core::MutableContext ctx, const absl::Span<const ParseError> parseErrors,
               bool directlyDesugar)
        : parser(parser), ctx(ctx), parseErrors(parseErrors), directlyDesugar(directlyDesugar), uniqueCounterStorage(1),
          uniqueCounter(&this->uniqueCounterStorage) {}

    int nextUniqueID() {
        return *uniqueCounter += 1;
    }

    // Translates the given AST from Prism's node types into the equivalent AST in Sorbet's legacy parser node types.
    std::unique_ptr<parser::Node> translate(pm_node_t *node);

private:
    // This private constructor is used for creating child translators with modified context.
    // uniqueCounterStorage is passed as the minimum integer value and is never used
    Translator(const Translator &parent, bool isInMethodDef)
        : parser(parent.parser), ctx(parent.ctx), parseErrors(parent.parseErrors),
          directlyDesugar(parent.directlyDesugar), uniqueCounterStorage(std::numeric_limits<int>::min()),
          uniqueCounter(parent.uniqueCounter), isInMethodDef(isInMethodDef) {}

    template <typename SorbetNode, typename... TArgs>
    std::unique_ptr<parser::Node> make_node_with_expr(ast::ExpressionPtr desugaredExpr, TArgs &&...args);

    template <typename SorbetNode, typename... TArgs>
    std::unique_ptr<parser::Node> make_unsupported_node(TArgs &&...args);

    core::LocOffsets translateLoc(pm_location_t loc);

    parser::NodeVec translateMulti(pm_node_list prismNodes);
    void translateMultiInto(NodeVec &sorbetNodes, absl::Span<pm_node_t *> prismNodes);

    NodeVec translateArguments(pm_arguments_node *node, pm_node *blockArgumentNode = nullptr);
    parser::NodeVec translateKeyValuePairs(pm_node_list_t elements);
    static bool isKeywordHashElement(sorbet::parser::Node *nd);
    std::unique_ptr<parser::Node> translateCallWithBlock(pm_node_t *prismBlockOrLambdaNode,
                                                         std::unique_ptr<parser::Node> sendNode);
    std::unique_ptr<parser::Node> translateRescue(pm_rescue_node *prismRescueNode,
                                                  std::unique_ptr<parser::Node> beginNode,
                                                  std::unique_ptr<parser::Node> elseNode);
    std::unique_ptr<parser::Node> translateStatements(pm_statements_node *stmtsNode, bool inlineIfSingle = true);

    std::unique_ptr<parser::Regopt> translateRegexpOptions(pm_location_t closingLoc);
    std::unique_ptr<parser::Regexp> translateRegexp(pm_string_t unescaped, core::LocOffsets location,
                                                    pm_location_t closingLoc);

    template <typename PrismNode> std::unique_ptr<parser::Mlhs> translateMultiTargetLhs(PrismNode *);

    template <typename PrismAssignmentNode, typename SorbetLHSNode>
    std::unique_ptr<parser::Assign> translateAssignment(pm_node_t *node);

    template <typename PrismAssignmentNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
    std::unique_ptr<SorbetAssignmentNode> translateOpAssignment(pm_node_t *node);

    template <typename PrismLhsNode, typename SorbetLHSNode>
    std::unique_ptr<parser::Node> translateConst(PrismLhsNode *node, bool replaceWithDynamicConstAssign = false);
    core::NameRef translateConstantName(pm_constant_id_t constantId);

    // Pattern-matching
    // ... variations of the main translation functions for pattern-matching related nodes.
    std::unique_ptr<parser::Node> patternTranslate(pm_node_t *node);
    parser::NodeVec patternTranslateMulti(pm_node_list prismNodes);
    void patternTranslateMultiInto(NodeVec &sorbetNodes, absl::Span<pm_node_t *> prismNodes);

    std::string_view sliceLocation(pm_location_t loc);

    void reportError(core::LocOffsets loc, const std::string &message);

    // Context management helpers. These return a copy of `this` with some change to the context.
    Translator enterMethodDef();
};

} // namespace sorbet::parser::Prism
#endif // SORBET_PARSER_PRISM_TRANSLATOR_H
