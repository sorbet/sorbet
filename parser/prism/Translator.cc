#include "Translator.h"
#include "Helpers.h"

#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/desugar/DuplicateHashKeyCheck.h"
#include "core/errors/desugar.h"

#include "absl/strings/str_replace.h"

using namespace std;

namespace sorbet::parser::Prism {

using namespace std::literals::string_view_literals;
using sorbet::ast::MK;
using ExpressionPtr = sorbet::ast::ExpressionPtr;

class ExprOnly final : public Node {
    ast::ExpressionPtr desugaredExpr;

public:
    ExprOnly(ast::ExpressionPtr desugaredExpr, core::LocOffsets loc)
        : Node(loc), desugaredExpr(std::move(desugaredExpr)) {
        ENFORCE(this->desugaredExpr != nullptr, "Can't create ExprOnly with a null desugaredExpr.");
    }
    virtual ~ExprOnly() = default;

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const final {
        Exception::raise("Not implemented");
    }

    virtual std::string toJSON(const core::GlobalState &gs, int tabs = 0) final {
        Exception::raise("Not implemented");
    }

    virtual std::string toJSONWithLocs(const core::GlobalState &gs, core::FileRef file, int tabs = 0) final {
        Exception::raise("Not implemented");
    }

    virtual std::string toWhitequark(const core::GlobalState &gs, int tabs = 0) final {
        Exception::raise("Not implemented");
    }

    virtual std::string nodeName() const final {
        Exception::raise("Not implemented");
    }

    virtual ast::ExpressionPtr takeDesugaredExpr() final {
        ENFORCE(this->desugaredExpr != nullptr,
                "Tried to call make a second call to `takeDesugaredExpr()` on a ExprOnly");

        // We know each `NodeAndExpr` object's `takeDesugaredExpr()` will be called at most once, either:
        // 1. When its parent node is being translated in `prism/Translator.cc`,
        //    and this value is used to create that parent's expr.
        // 2. When this node is visted by `node2TreeImpl` in `PrismDesugar.cc`,
        //    and this value is called from the `ExprOnly` case
        //
        // Because of this, we don't need to make any copies here. Just move this value out,
        // and hand exclusive ownership to the caller.
        return move(this->desugaredExpr);
    }

    virtual bool hasDesugaredExpr() final {
        return this->desugaredExpr != nullptr;
    }

    virtual const ast::ExpressionPtr &peekDesugaredExpr() const final {
        return this->desugaredExpr;
    }
};

// Represents a desugared block argument. Valid cases can have:
// - Just a block pass (e.g., `foo(&block)`)
// - Just a literal block (e.g., `foo { }`)
// - Neither
//
// We also need to model the invalid case, where both a block pass and literal block
// (e.g., `foo(...) { }` or `foo(&block) { }`)
class Translator::DesugaredBlockArgument {
public:
    // The literal block, if any, e.g. `{ ... }` or `do ... end`
    ast::ExpressionPtr literalBlockExpr;

    // The expression passed as a block, if any, e.g. the `block` in `a.map(&block)`
    ast::ExpressionPtr blockPassExpr;

    // The location of the entire block pass, including the `&`.
    //     a.map(&:foo)
    //           ^^^^^
    core::LocOffsets blockPassLoc;

private:
    // Hide the constructor, in favour of named factory methods, so that we can add validation logic if needed.
    DesugaredBlockArgument(ast::ExpressionPtr literalBlockExpr, ast::ExpressionPtr blockPassExpr,
                           core::LocOffsets blockPassLoc)
        : literalBlockExpr(move(literalBlockExpr)), blockPassExpr(move(blockPassExpr)), blockPassLoc(blockPassLoc) {}

public:
    // Move-only type
    DesugaredBlockArgument(DesugaredBlockArgument &&) = default;                // Move constructor
    DesugaredBlockArgument(const DesugaredBlockArgument &) = delete;            // Copy constructor
    DesugaredBlockArgument &operator=(DesugaredBlockArgument &&) = default;     // Move assignment
    DesugaredBlockArgument &operator=(const DesugaredBlockArgument &) = delete; // Copy assignment

    static DesugaredBlockArgument none() {
        return DesugaredBlockArgument(nullptr, nullptr, core::LocOffsets::none());
    }

    static DesugaredBlockArgument blockPass(ast::ExpressionPtr blockPassExpr, core::LocOffsets blockPassLoc) {
        return DesugaredBlockArgument(nullptr, move(blockPassExpr), blockPassLoc);
    }

    static DesugaredBlockArgument literalBlock(ast::ExpressionPtr block) {
        return DesugaredBlockArgument(move(block), nullptr, core::LocOffsets::none());
    }

    static DesugaredBlockArgument both(ast::ExpressionPtr block, ast::ExpressionPtr blockPassExpr,
                                       core::LocOffsets blockPassLoc) {
        return DesugaredBlockArgument(move(block), move(blockPassExpr), blockPassLoc);
    }

    bool exists() const {
        return literalBlockExpr || blockPassExpr;
    }

    bool hasBlockPass() const {
        return blockPassExpr != nullptr;
    }

    bool hasLiteralBlock() const {
        return literalBlockExpr != nullptr;
    }
};

// Helper template to convert a pm_node_list to any store type.
// This is used to convert prism node lists to store types like ast::Array::ENTRY_store,
// ast::Send::ARGS_store, ast::InsSeq::STATS_store, etc.
template <typename StoreType> StoreType Translator::nodeListToStore(const pm_node_list &nodeList) {
    auto span = absl::MakeSpan(nodeList.nodes, nodeList.size);

    StoreType store;
    store.reserve(span.size());
    for (auto &element : span) {
        auto expr = desugar(element);
        store.emplace_back(move(expr));
    }
    return store;
}

// Collect pattern variable assignments from a pattern node (similar to desugarPatternMatchingVars in PrismDesugar.cc)
void Translator::collectPatternMatchingVarsPrism(ast::InsSeq::STATS_store &vars, pm_node_t *node) {
    if (node == nullptr) {
        return;
    }

    if (PM_NODE_TYPE_P(node, PM_LOCAL_VARIABLE_TARGET_NODE)) {
        auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
        auto loc = translateLoc(localVarTargetNode->base.location);
        auto val = MK::RaiseUnimplemented(loc);
        auto name = translateConstantName(localVarTargetNode->name);
        vars.emplace_back(MK::Assign(loc, name, move(val)));
    } else if (PM_NODE_TYPE_P(node, PM_SPLAT_NODE)) {
        // MatchRest in array patterns - recurse on the expression (variable being splatted into)
        auto splatNode = down_cast<pm_splat_node>(node);
        collectPatternMatchingVarsPrism(vars, splatNode->expression);
    } else if (PM_NODE_TYPE_P(node, PM_ASSOC_SPLAT_NODE)) {
        // MatchRest in hash patterns - recurse on the value
        auto assocSplatNode = down_cast<pm_assoc_splat_node>(node);
        collectPatternMatchingVarsPrism(vars, assocSplatNode->value);
    } else if (PM_NODE_TYPE_P(node, PM_ASSOC_NODE)) {
        // Pair in hash pattern - only recurse on the value (key is a symbol, not a variable)
        auto assocNode = down_cast<pm_assoc_node>(node);
        // Special handling for implicit hash pattern keys like `n1:` which means `n1: n1`
        // Legacy parser uses the assoc node's location (including colon) for the variable
        if (PM_NODE_TYPE_P(assocNode->value, PM_IMPLICIT_NODE)) {
            auto implicitNode = down_cast<pm_implicit_node>(assocNode->value);
            if (PM_NODE_TYPE_P(implicitNode->value, PM_LOCAL_VARIABLE_TARGET_NODE)) {
                auto localVarTargetNode = down_cast<pm_local_variable_target_node>(implicitNode->value);
                auto loc = translateLoc(assocNode->base.location); // Use assoc node's location
                auto name = translateConstantName(localVarTargetNode->name);
                auto val = MK::RaiseUnimplemented(loc);
                vars.emplace_back(MK::Assign(loc, name, move(val)));
                return;
            }
        }
        collectPatternMatchingVarsPrism(vars, assocNode->value);
    } else if (PM_NODE_TYPE_P(node, PM_CAPTURE_PATTERN_NODE)) {
        // MatchAs - use the target's location, not the whole pattern's location
        auto matchAsNode = down_cast<pm_capture_pattern_node>(node);
        auto loc = translateLoc(matchAsNode->target->base.location);
        auto name = translateConstantName(matchAsNode->target->name);
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, name, move(val)));
        collectPatternMatchingVarsPrism(vars, matchAsNode->value);
    } else if (PM_NODE_TYPE_P(node, PM_ARRAY_PATTERN_NODE)) {
        auto arrayPatternNode = down_cast<pm_array_pattern_node>(node);
        // Skip ConstPattern - legacy parser doesn't collect variables from ConstPattern
        if (arrayPatternNode->constant != nullptr) {
            return;
        }
        auto requireds = absl::MakeSpan(arrayPatternNode->requireds.nodes, arrayPatternNode->requireds.size);
        for (auto &elt : requireds) {
            collectPatternMatchingVarsPrism(vars, elt);
        }
        // Process rest element (skip implicit rest nodes as they don't bind variables)
        if (arrayPatternNode->rest != nullptr && !PM_NODE_TYPE_P(arrayPatternNode->rest, PM_IMPLICIT_REST_NODE)) {
            collectPatternMatchingVarsPrism(vars, arrayPatternNode->rest);
        }
        auto posts = absl::MakeSpan(arrayPatternNode->posts.nodes, arrayPatternNode->posts.size);
        for (auto &elt : posts) {
            collectPatternMatchingVarsPrism(vars, elt);
        }
    } else if (PM_NODE_TYPE_P(node, PM_HASH_PATTERN_NODE)) {
        auto hashPatternNode = down_cast<pm_hash_pattern_node>(node);
        // Skip ConstPattern - legacy parser doesn't collect variables from ConstPattern
        if (hashPatternNode->constant != nullptr) {
            return;
        }
        auto elements = absl::MakeSpan(hashPatternNode->elements.nodes, hashPatternNode->elements.size);
        for (auto &elt : elements) {
            collectPatternMatchingVarsPrism(vars, elt);
        }
        // Process rest element
        collectPatternMatchingVarsPrism(vars, hashPatternNode->rest);
    } else if (PM_NODE_TYPE_P(node, PM_ALTERNATION_PATTERN_NODE)) {
        auto alternationPatternNode = down_cast<pm_alternation_pattern_node>(node);
        collectPatternMatchingVarsPrism(vars, alternationPatternNode->left);
        collectPatternMatchingVarsPrism(vars, alternationPatternNode->right);
    } else if (PM_NODE_TYPE_P(node, PM_IF_NODE)) {
        // Pattern with if guard - the actual pattern is inside statements
        auto ifNode = down_cast<pm_if_node>(node);
        if (ifNode->statements != nullptr && ifNode->statements->body.size > 0) {
            collectPatternMatchingVarsPrism(vars, ifNode->statements->body.nodes[0]);
        }
    } else if (PM_NODE_TYPE_P(node, PM_UNLESS_NODE)) {
        // Pattern with unless guard - the actual pattern is inside statements
        auto unlessNode = down_cast<pm_unless_node>(node);
        if (unlessNode->statements != nullptr && unlessNode->statements->body.size > 0) {
            collectPatternMatchingVarsPrism(vars, unlessNode->statements->body.nodes[0]);
        }
    }
}

// Desugar `in` and `=>` oneline pattern matching (mirrors desugarOnelinePattern in Desugar.cc)
ast::ExpressionPtr Translator::desugarOnelinePattern(core::LocOffsets loc, pm_node_t *match) {
    auto matchExpr = MK::RaiseUnimplemented(loc);
    auto bodyExpr = MK::RaiseUnimplemented(loc);
    auto elseExpr = MK::EmptyTree();

    ast::InsSeq::STATS_store vars;
    collectPatternMatchingVarsPrism(vars, match);

    if (!vars.empty()) {
        auto matchLoc = match != nullptr ? translateLoc(match->location) : loc;
        bodyExpr = MK::InsSeq(matchLoc, move(vars), move(bodyExpr));
    }

    return MK::If(loc, move(matchExpr), move(bodyExpr), move(elseExpr));
}

ast::ExpressionPtr Translator::make_unsupported_node(core::LocOffsets loc, std::string_view nodeName) const {
    if (auto e = ctx.beginIndexerError(loc, core::errors::Desugar::UnsupportedNode)) {
        e.setHeader("Unsupported node type `{}`", nodeName);
    }

    return MK::EmptyTree();
}

// Indicates that a particular code path should never be reached, with an explanation of why.
// Throws a `sorbet::SorbetException` when triggered to help with debugging.
template <typename... TArgs>
[[noreturn]] void unreachable(fmt::format_string<TArgs...> reasonFormatStr, TArgs &&...args) {
    Exception::raise(reasonFormatStr, forward<TArgs>(args)...);
}

// Helper function to check if an AST expression is a string literal
bool isStringLit(const ast::ExpressionPtr &expr) {
    if (auto lit = ast::cast_tree<ast::Literal>(expr)) {
        return lit->isString();
    }
    return false;
}

// Flattens the key/value pairs from the Kwargs Hash into the destination container.
// If Kwargs Hash contains any splats, we skip the flattening and desugar the hash as-is.
template <typename Container>
void Translator::flattenKwargs(pm_keyword_hash_node *kwargsHashNode, Container &destination) {
    ENFORCE(kwargsHashNode != nullptr);

    auto elements = absl::MakeSpan(kwargsHashNode->elements.nodes, kwargsHashNode->elements.size);

    // Check if there are any splats - if so, can't flatten
    bool hasKwsplat = absl::c_any_of(elements, [](auto *node) { return PM_NODE_TYPE_P(node, PM_ASSOC_SPLAT_NODE); });

    if (hasKwsplat) {
        // Desugar the whole hash using desugarKeyValuePairs which handles kwsplats properly
        auto loc = translateLoc(kwargsHashNode->base.location);
        destination.emplace_back(desugarKeyValuePairs(loc, kwargsHashNode->elements));
        return;
    }

    // Flatten each key/value pair directly
    for (auto *element : elements) {
        ENFORCE(PM_NODE_TYPE_P(element, PM_ASSOC_NODE));
        auto *assoc = down_cast<pm_assoc_node>(element);

        // Special handling for symbol keys with trailing colon (like `a: 1` instead of `:a => 1`)
        if (PM_NODE_TYPE_P(assoc->key, PM_SYMBOL_NODE)) {
            auto *symbolNode = down_cast<pm_symbol_node>(assoc->key);

            // If opening_loc is null, the symbol has a trailing colon - drop it from the location
            if (symbolNode->opening_loc.start == nullptr) {
                auto symbolLoc = translateLoc(symbolNode->base.location.start, symbolNode->base.location.end - 1);
                auto [symbolContent, _] = translateSymbol(symbolNode);
                destination.emplace_back(MK::Symbol(symbolLoc, symbolContent));
                destination.emplace_back(desugar(assoc->value));
                continue;
            }
        }

        destination.emplace_back(desugar(assoc->key));
        destination.emplace_back(desugar(assoc->value));
    }
}

// Helper function to merge multiple string literals into one
ast::ExpressionPtr mergeStrings(core::MutableContext ctx, core::LocOffsets loc,
                                absl::InlinedVector<ast::ExpressionPtr, 4> stringsAccumulated) {
    if (stringsAccumulated.size() == 1) {
        return move(stringsAccumulated[0]);
    } else {
        std::string result;
        for (const auto &expr : stringsAccumulated) {
            result += ast::cast_tree_nonnull<ast::Literal>(expr).asString().shortName(ctx);
        }
        return MK::String(loc, ctx.state.enterNameUTF8(result));
    }
}

// Extract the content and location of a Symbol node.
// This is handy for `desugarSymbolProc`, where it saves us from needing to dig and
// cast to extract this info out of an `ast::Literal`.
pair<core::NameRef, core::LocOffsets> Translator::translateSymbol(pm_symbol_node *symbol) {
    auto location = translateLoc(symbol->base.location);

    auto unescaped = &symbol->unescaped;
    // TODO: can these have different encodings?
    auto content = ctx.state.enterNameUTF8(parser.extractString(unescaped));

    return make_pair(content, location);
}

ast::ExpressionPtr Translator::desugarDString(core::LocOffsets loc, pm_node_list prismNodeList) {
    if (prismNodeList.size == 0) {
        return MK::String(loc, core::Names::empty());
    }

    absl::InlinedVector<ast::ExpressionPtr, 4> stringsAccumulated;
    ast::Send::ARGS_store interpArgs;

    bool allStringsSoFar = true;

    auto prismNodes = absl::MakeSpan(prismNodeList.nodes, prismNodeList.size);
    for (pm_node *prismNode : prismNodes) {
        auto expr = desugar(prismNode);

        if (allStringsSoFar && isStringLit(expr)) {
            stringsAccumulated.emplace_back(move(expr));
        } else {
            if (allStringsSoFar) {
                // Transition from all strings to mixed content
                allStringsSoFar = false;
                if (!stringsAccumulated.empty()) {
                    auto mergedStrings = mergeStrings(ctx, loc, move(stringsAccumulated));
                    interpArgs.emplace_back(move(mergedStrings));
                }
            }
            interpArgs.emplace_back(move(expr));
        }
    }

    if (allStringsSoFar) {
        return mergeStrings(ctx, loc, move(stringsAccumulated));
    }

    auto recv = MK::Magic(loc);
    return MK::Send(loc, move(recv), core::Names::stringInterpolate(), loc.copyWithZeroLength(),
                    static_cast<uint16_t>(interpArgs.size()), move(interpArgs));
}

// Desugar multiple left hand side assignments into a sequence of assignments
//
// Considering this example:
// ```rb
// arr = [1, 2, 3]
// a, *b = arr
// ```
//
// We desugar the assignment `a, *b = arr` into:
// ```rb
// tmp = ::<Magic>.expandSplat(arr, 1, 0)
// a = tmp[0]
// b = tmp.to_ary
// ```
//
// While calling `to_ary` doesn't return the correct value if we were to execute this code,
// it returns the correct type from a static point of view.
template <typename PrismNode>
ast::ExpressionPtr Translator::desugarMlhs(core::LocOffsets loc, PrismNode *lhs, ast::ExpressionPtr rhs) {
    static_assert(is_same_v<PrismNode, pm_multi_target_node> || is_same_v<PrismNode, pm_multi_write_node>);

    ast::InsSeq::STATS_store stats;

    core::NameRef tempRhs = nextUniqueDesugarName(core::Names::assignTemp());
    core::NameRef tempExpanded = nextUniqueDesugarName(core::Names::assignTemp());

    int i = 0;
    int before = 0, after = 0;
    bool didSplat = false;
    auto zloc = loc.copyWithZeroLength();

    auto lefts = absl::MakeSpan(lhs->lefts.nodes, lhs->lefts.size);
    auto rights = absl::MakeSpan(lhs->rights.nodes, lhs->rights.size);
    bool hasSplat = lhs->rest && PM_NODE_TYPE_P(lhs->rest, PM_SPLAT_NODE);
    size_t totalSize = lefts.size() + (hasSplat ? 1 : 0) + rights.size();

    auto processTarget = [this, &didSplat, &stats, &i, &after, &before, totalSize, zloc, tempExpanded](pm_node_t *c) {
        if (PM_NODE_TYPE_P(c, PM_SPLAT_NODE)) {
            auto *splat = down_cast<pm_splat_node>(c);
            ENFORCE(!didSplat, "did splat already");
            didSplat = true;

            int left = i;
            int right = totalSize - left - 1;

            if (splat->expression) {
                ast::ExpressionPtr lh = desugar(splat->expression);

                if (right == 0) {
                    right = 1;
                }
                auto lhloc = lh.loc();
                auto zlhloc = lhloc.copyWithZeroLength();
                // Calling `to_ary` is not faithful to the runtime behavior,
                // but that it is faithful to the expected static type-checking behavior.
                auto ary = MK::Send0(zloc, MK::Local(zloc, tempExpanded), core::Names::toAry(), zlhloc);
                stats.emplace_back(MK::Assign(lhloc, move(lh), move(ary)));
            }
            i = -right;
        } else {
            if (didSplat) {
                ++after;
            } else {
                ++before;
            }

            auto cloc = translateLoc(c->location);
            auto zcloc = cloc.copyWithZeroLength();
            auto val =
                MK::Send1(zcloc, MK::Local(zcloc, tempExpanded), core::Names::squareBrackets(), zloc, MK::Int(zloc, i));

            if (PM_NODE_TYPE_P(c, PM_MULTI_TARGET_NODE)) {
                auto *mlhs = down_cast<pm_multi_target_node>(c);
                stats.emplace_back(desugarMlhs(cloc, mlhs, move(val)));
            } else {
                ast::ExpressionPtr lh = desugar(c);
                if (auto restParam = ast::cast_tree<ast::RestParam>(lh)) {
                    if (auto e =
                            ctx.beginIndexerError(lh.loc(), core::errors::Desugar::UnsupportedRestArgsDestructure)) {
                        e.setHeader("Unsupported rest args in destructure");
                    }
                    lh = move(restParam->expr);
                }

                auto lhloc = lh.loc();
                stats.emplace_back(MK::Assign(lhloc, move(lh), move(val)));
            }

            i++;
        }
    };

    for (auto *c : lefts) {
        processTarget(c);
    }
    if (hasSplat) {
        processTarget(lhs->rest);
    }
    for (auto *c : rights) {
        processTarget(c);
    }

    auto expanded = MK::Send3(loc, MK::Magic(loc), core::Names::expandSplat(), zloc, MK::Local(loc, tempRhs),
                              MK::Int(loc, before), MK::Int(loc, after));
    stats.insert(stats.begin(), MK::Assign(loc, tempExpanded, move(expanded)));
    stats.insert(stats.begin(), MK::Assign(loc, tempRhs, move(rhs)));

    // Regardless of how we destructure an assignment, Ruby evaluates the expression to the entire right hand side,
    // not any individual component of the destructured assignment.
    return MK::InsSeq(loc, move(stats), MK::Local(loc, tempRhs));
}

std::pair</* param */ ast::ExpressionPtr, /* multi-assign statement */ ast::ExpressionPtr>
Translator::desugarMlhsParam(core::LocOffsets loc, pm_multi_target_node *lhs) {
    core::NameRef destructureParam = nextUniqueDesugarName(core::Names::destructureArg());
    auto param = MK::Local(loc, destructureParam);
    auto destructuringExpr = desugarMlhs(loc, lhs, MK::Local(loc, destructureParam));

    return {move(param), move(destructuringExpr)};
}

// Helper to check if an ExpressionPtr represents a T.let call
ast::Send *asTLet(ExpressionPtr &arg) {
    auto send = ast::cast_tree<ast::Send>(arg);
    if (send == nullptr || send->fun != core::Names::let() || send->numPosArgs() < 2) {
        return nullptr;
    }

    if (!ast::MK::isTApproximate(send->recv)) {
        return nullptr;
    }

    return send;
}

// widen the type from `parser::OpAsgn` to `parser::Node` to handle `make_node_with_expr` correctly.
// TODO: narrow the type back after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
// The location is the location of the whole Prism assignment node.
template <typename PrismAssignmentNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
ast::ExpressionPtr Translator::translateAnyOpAssignment(PrismAssignmentNode *node, core::LocOffsets location,
                                                        ast::ExpressionPtr lhs) {
    auto rhs = desugar(node->value);

    if constexpr (is_same_v<SorbetAssignmentNode, parser::AndAsgn>) {
        return translateAndOrAssignment<parser::AndAsgn>(location, move(lhs), move(rhs));
    } else if constexpr (is_same_v<SorbetAssignmentNode, parser::OrAsgn>) {
        return translateAndOrAssignment<parser::OrAsgn>(location, move(lhs), move(rhs));
    } else if constexpr (is_same_v<SorbetAssignmentNode, parser::OpAsgn>) {
        return translateOpAssignment<SorbetAssignmentNode, PrismAssignmentNode>(node, location, move(lhs), move(rhs));
    } else {
        static_assert(
            always_false_v<SorbetAssignmentNode>,
            "Invalid operator node type. Must be one of `parser::OpAssign`, `parser::AndAsgn` or `parser::OrAsgn`.");
    }
}

// The location is the location of the whole Prism assignment node.
template <typename PrismAssignmentNode, typename SorbetAssignmentNode>
ast::ExpressionPtr Translator::translateIndexAssignment(pm_node_t *untypedNode, core::LocOffsets location) {
    auto node = down_cast<PrismAssignmentNode>(untypedNode);

    // The LHS location includes the receiver and the `[]`, but not the `=` or rhs.
    // self.example[k] = v
    // ^^^^^^^^^^^^^^^
    auto lhsLoc = translateLoc(node->receiver->location.start, node->closing_loc.end);

    auto receiver = desugar(node->receiver);

    // Handle operator assignment to an indexed expression, like `a[0] += 1`
    auto openingLoc = translateLoc(node->opening_loc);
    auto lBracketLoc = core::LocOffsets{openingLoc.beginLoc, openingLoc.endLoc - 1};

    auto args = desugarArguments<ast::Send::ARGS_store>(node->arguments);
    auto argsSize = args.size(); // Grab the size before moving out of `args`

    // Desugar `x[i] = y, z` to `x.[]=(i, y, z)`
    auto lhs = MK::Send(lhsLoc, move(receiver), core::Names::squareBrackets(), lBracketLoc, argsSize, move(args));

    return translateAnyOpAssignment<PrismAssignmentNode, SorbetAssignmentNode, void>(node, location, move(lhs));
}

// The location is the location of the whole Prism assignment node.
template <typename SorbetAssignmentNode>
ast::ExpressionPtr Translator::translateAndOrAssignment(core::LocOffsets location, ast::ExpressionPtr lhs,
                                                        ast::ExpressionPtr rhs) {
    const auto isOrAsgn = is_same_v<SorbetAssignmentNode, parser::OrAsgn>;
    const auto isAndAsgn = is_same_v<SorbetAssignmentNode, parser::AndAsgn>;
    static_assert(isOrAsgn || isAndAsgn);

    if (preserveConcreteSyntax) {
        auto magicName = isAndAsgn ? core::Names::andAsgn() : core::Names::orAsgn();
        auto locZeroLen = location.copyWithZeroLength();

        // Desugar `x &&= y` to `<Magic>.&&=(x, y)` (likewise for `||=`)
        return MK::Send2(location, MK::Magic(locZeroLen), magicName, locZeroLen, move(lhs), move(rhs));
    }

    if (auto s = ast::cast_tree<ast::Send>(lhs)) {
        auto sendLoc = s->loc;
        auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(rhs));
        auto cond =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto tempResult = nextUniqueDesugarName(s->fun);
        stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));
        auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                             numPosAssgnArgs, move(assgnArgs), s->flags);
        auto elsep = MK::Local(sendLoc, tempResult);
        ExpressionPtr if_;
        if constexpr (isAndAsgn) {
            // Desugar `lhs &&= rhs` to `if (lhs) { lhs = rhs } else { lhs }`
            if_ = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
        } else {
            // OrAsgn: if (lhs) { lhs } else { lhs = rhs }
            if_ = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(elsep), move(body));
        }
        return MK::InsSeq(location, move(stats), move(if_));
    }

    if (isa_reference(lhs)) {
        auto lhsCopy = MK::cpRef(lhs);
        auto cond = MK::cpRef(lhs);

        // Check for T.let handling for instance and class variables in ||= assignments
        auto lhsIdentifier = ast::cast_tree<ast::UnresolvedIdent>(lhs);
        auto rhsIdentifier = ast::cast_tree<ast::UnresolvedIdent>(rhs);
        ENFORCE(lhsIdentifier);
        ENFORCE(rhsIdentifier);
        auto lhsIsIvar = lhsIdentifier->kind == ast::UnresolvedIdent::Kind::Instance;
        auto lhsIsCvar = lhsIdentifier->kind == ast::UnresolvedIdent::Kind::Class;
        auto rhsIsTLet = asTLet(rhs);

        ExpressionPtr assignExpr;
        if (isOrAsgn && (lhsIsIvar || lhsIsCvar) && rhsIsTLet) {
            // Special handling for ||= with T.let on instance/class variables
            // Save the original value before replacing it
            auto originalValue = rhsIsTLet->getPosArg(0).deepCopy();

            // Replace the first argument of T.let with the LHS variable
            rhsIsTLet->getPosArg(0) = MK::cpRef(lhs);

            // Generate pattern: { @z = T.let(@z, ...); <temp> = <original_value>; @z = <temp> }
            auto decl = MK::Assign(location, MK::cpRef(lhs), move(rhs));

            // Create a temporary variable and assign the original value to it
            core::NameRef tempName = nextUniqueDesugarName(core::Names::statTemp());
            auto tempAssign = MK::Assign(location, tempName, move(originalValue));

            // Final assignment from temp to LHS
            auto finalAssign = MK::Assign(location, MK::cpRef(lhs), MK::Local(location, tempName));

            ast::InsSeq::STATS_store stats;
            stats.emplace_back(move(decl));
            stats.emplace_back(move(tempAssign));

            assignExpr = MK::InsSeq(location, move(stats), move(finalAssign));
        } else {
            assignExpr = MK::Assign(location, MK::cpRef(lhs), move(rhs));
        }

        if constexpr (isAndAsgn) {
            // AndAsgn: if (lhs) { lhs = rhs } else { lhs }
            return MK::If(location, move(cond), move(assignExpr), move(lhsCopy));
        } else {
            // OrAsgn: if (lhs) { lhs } else { lhs = rhs }
            return MK::If(location, move(cond), move(lhsCopy), move(assignExpr));
        }
    }

    if (ast::isa_tree<ast::UnresolvedConstantLit>(lhs)) {
        if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
            e.setHeader("Constant reassignment is not supported");
        }
        return MK::EmptyTree();
    }

    if (ast::isa_tree<ast::InsSeq>(lhs)) {
        auto i = ast::cast_tree<ast::InsSeq>(lhs);
        auto ifExpr = ast::cast_tree<ast::If>(i->expr);
        if (!ifExpr) {
            Exception::raise("Unexpected left-hand side of op=: please file an issue");
        }
        auto s = ast::cast_tree<ast::Send>(ifExpr->elsep);
        if (!s) {
            Exception::raise("Unexpected left-hand side of op=: please file an issue");
        }
        auto sendLoc = s->loc;
        auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(rhs));
        auto cond =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto tempResult = nextUniqueDesugarName(s->fun);
        stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));
        auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                             numPosAssgnArgs, move(assgnArgs), s->flags);
        auto elsep = MK::Local(sendLoc, tempResult);
        auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
        return MK::InsSeq(location, move(stats), move(iff));
    }

    Exception::raise("the LHS has been desugared to something we haven't expected: {}", lhs.toString(ctx));
}

Translator::OpAsgnScaffolding Translator::copyArgsForOpAsgn(ast::Send *s) {
    // This is for storing the temporary assignments followed by the final update. In the case that we have other
    // arguments to the send (e.g. in the case of x.y[z] += 1) we'll want to store the other parameters (z) in a
    // temporary as well, producing a sequence like
    //
    //   { $arg = z; $tmp = x.y[$arg]; x.y[$arg] = $tmp + 1 }
    //
    // This means we'll always need statements for as many arguments as the send has, plus two more: one for the
    // temporary assignment and the last for the actual update we're desugaring.
    ENFORCE(!s->hasKwArgs() && !s->hasBlock());
    const auto numPosArgs = s->numPosArgs();
    ast::InsSeq::STATS_store stats;
    stats.reserve(numPosArgs + 2);
    core::NameRef tempRecv = nextUniqueDesugarName(s->fun);
    stats.emplace_back(MK::Assign(s->loc, tempRecv, move(s->recv)));
    ast::Send::ARGS_store readArgs;
    ast::Send::ARGS_store assgnArgs;
    // these are the arguments for the first send, e.g. x.y(). The number of arguments should be identical to
    // whatever we saw on the LHS.
    readArgs.reserve(numPosArgs);
    // these are the arguments for the second send, e.g. x.y=(val). That's why we need the space for the extra
    // argument here: to accommodate the call to field= instead of just field.
    assgnArgs.reserve(numPosArgs + 1);

    for (auto &arg : s->posArgs()) {
        auto argLoc = arg.loc();
        core::NameRef name = nextUniqueDesugarName(s->fun);
        stats.emplace_back(MK::Assign(argLoc, name, move(arg)));
        readArgs.emplace_back(MK::Local(argLoc, name));
        assgnArgs.emplace_back(MK::Local(argLoc, name));
    }

    return {tempRecv, move(stats), numPosArgs, move(readArgs), move(assgnArgs)};
}

// The location is the location of the whole Prism assignment node.
template <typename SorbetAssignmentNode, typename PrismAssignmentNode>
ast::ExpressionPtr Translator::translateOpAssignment(PrismAssignmentNode *node, core::LocOffsets location,
                                                     ast::ExpressionPtr lhs, ast::ExpressionPtr rhs) {
    // `OpAsgn` assign needs more information about the specific operator here, so it gets special handling here.
    auto opLoc = translateLoc(node->binary_operator_loc);
    auto op = translateConstantName(node->binary_operator);

    if (preserveConcreteSyntax) {
        auto magicName = core::Names::opAsgn();
        auto locZeroLen = location.copyWithZeroLength();
        return MK::Send2(location, MK::Magic(locZeroLen), magicName, locZeroLen, move(lhs), move(rhs));
    }

    if (ast::isa_tree<ast::Send>(lhs)) {
        auto s = ast::cast_tree<ast::Send>(lhs);
        auto sendLoc = s->loc;
        auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);

        // Create the read operation: obj.method() or obj[index]
        auto prevValue =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);

        // Apply the operation: prevValue op rhs
        auto newValue = MK::Send1(sendLoc, move(prevValue), op, opLoc, move(rhs));

        // Add the new value to the assignment arguments
        assgnArgs.emplace_back(move(newValue));
        auto numPosAssgnArgs = numPosArgs + 1;

        // Create the assignment operation: obj.method=(newValue) or obj[]=(index, newValue)
        auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                            numPosAssgnArgs, move(assgnArgs), s->flags);

        return MK::InsSeq(location, move(stats), move(res));
    }

    if (isa_reference(lhs)) {
        auto lhsCopy = MK::cpRef(lhs);
        auto callOp = MK::Send1(location, move(lhs), op, opLoc, move(rhs));
        return MK::Assign(location, move(lhsCopy), move(callOp));
    }

    if (ast::isa_tree<ast::UnresolvedConstantLit>(lhs)) {
        if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
            e.setHeader("Constant reassignment is not supported");
        }
        return MK::EmptyTree();
    }

    if (auto i = ast::cast_tree<ast::InsSeq>(lhs)) {
        // if this is an InsSeq, then is probably the result of a safe send (i.e. an expression of the form
        // x&.y on the LHS) which means it'll take the rough shape:
        //   { $temp = x; if $temp == nil then nil else $temp.y }
        // on the LHS. We want to insert the y= into the if-expression at the end, like:
        //   { $temp = x; if $temp == nil then nil else { $t2 = $temp.y; $temp.y = $t2 op RHS } }
        // that means we first need to find out whether the final expression is an If...
        auto ifExpr = ast::cast_tree<ast::If>(i->expr);
        if (!ifExpr) {
            Exception::raise("Unexpected left-hand side of op=: please file an issue");
        }
        // and if so, find out whether the else-case is a send...
        auto s = ast::cast_tree<ast::Send>(ifExpr->elsep);
        if (!s) {
            Exception::raise("Unexpected left-hand side of op=: please file an issue");
        }
        // Similar to Send handling above but specialized for InsSeq structure modification
        auto sendLoc = s->loc;
        auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);
        auto prevValue =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto newValue = MK::Send1(sendLoc, move(prevValue), op, opLoc, move(rhs));
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(newValue));

        auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                            numPosAssgnArgs, move(assgnArgs), s->flags);
        auto wrapped = MK::InsSeq(location, move(stats), move(res));
        ifExpr->elsep = move(wrapped);
        return lhs;
    }

    auto s = fmt::format("the LHS has been desugared to something we haven't expected: {}", lhs.toString(ctx));
    Exception::raise(s);
}

// Desugar &&= or ||= for a simple reference LHS (local, instance, class, global variable).
// For `x &&= y`: `if x then x = y else x end`
// For `x ||= y`: `if x then x else x = y end`
template <OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarAndOrReference(core::LocOffsets location, ast::ExpressionPtr lhs,
                                                     ast::ExpressionPtr rhs, bool isIvarOrCvar) {
    static_assert(Kind == OpAssignKind::And || Kind == OpAssignKind::Or,
                  "desugarAndOrReference only handles And and Or assignments");

    constexpr bool isOrAsgn = (Kind == OpAssignKind::Or);

    auto lhsCopy = MK::cpRef(lhs);
    auto cond = MK::cpRef(lhs);

    // Check for T.let handling for instance and class variables in ||= assignments
    auto rhsIsTLet = asTLet(rhs);

    ExpressionPtr assignExpr;
    if constexpr (isOrAsgn) {
        if (isIvarOrCvar && rhsIsTLet) {
            // Special handling for ||= with T.let on instance/class variables
            // Save the original value before replacing it
            auto originalValue = rhsIsTLet->getPosArg(0).deepCopy();

            // Replace the first argument of T.let with the LHS variable
            rhsIsTLet->getPosArg(0) = MK::cpRef(lhs);

            // Generate pattern: { @z = T.let(@z, ...); <temp> = <original_value>; @z = <temp> }
            auto decl = MK::Assign(location, MK::cpRef(lhs), move(rhs));

            // Create a temporary variable and assign the original value to it
            core::NameRef tempName = nextUniqueDesugarName(core::Names::statTemp());
            auto tempAssign = MK::Assign(location, tempName, move(originalValue));

            // Final assignment from temp to LHS
            auto finalAssign = MK::Assign(location, MK::cpRef(lhs), MK::Local(location, tempName));

            ast::InsSeq::STATS_store stats;
            stats.emplace_back(move(decl));
            stats.emplace_back(move(tempAssign));

            assignExpr = MK::InsSeq(location, move(stats), move(finalAssign));
        } else {
            assignExpr = MK::Assign(location, MK::cpRef(lhs), move(rhs));
        }
    } else {
        assignExpr = MK::Assign(location, MK::cpRef(lhs), move(rhs));
    }

    if constexpr (Kind == OpAssignKind::And) {
        // AndAsgn: if (lhs) { lhs = rhs } else { lhs }
        return MK::If(location, move(cond), move(assignExpr), move(lhsCopy));
    } else {
        // OrAsgn: if (lhs) { lhs } else { lhs = rhs }
        return MK::If(location, move(cond), move(lhsCopy), move(assignExpr));
    }
}

// Desugar operator assignment (+=, -=, etc.) for a simple reference LHS.
// For `x += y`: `x = x + y`
ast::ExpressionPtr Translator::desugarOpReference(core::LocOffsets location, ast::ExpressionPtr lhs, core::NameRef op,
                                                  core::LocOffsets opLoc, ast::ExpressionPtr rhs) {
    auto lhsCopy = MK::cpRef(lhs);
    auto callOp = MK::Send1(location, move(lhs), op, opLoc, move(rhs));
    return MK::Assign(location, move(lhsCopy), move(callOp));
}

// Desugar compound assignment when LHS is a Send expression.
// For `recv.method &&= val`: `{ tmp = recv; result = tmp.method; if result then tmp.method= val else result }`
// For `recv.method ||= val`: `{ tmp = recv; result = tmp.method; if result then result else tmp.method= val }`
// For `recv.method += val`:  `{ tmp = recv; tmp.method= tmp.method + val }`
template <OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarOpAssignSend(core::LocOffsets location, ast::Send *s, ast::ExpressionPtr rhs,
                                                   core::NameRef op, core::LocOffsets opLoc) {
    auto sendLoc = s->loc;
    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);

    if constexpr (Kind == OpAssignKind::And || Kind == OpAssignKind::Or) {
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(rhs));
        auto cond =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto tempResult = nextUniqueDesugarName(s->fun);
        stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));
        auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                             numPosAssgnArgs, move(assgnArgs), s->flags);
        auto elsep = MK::Local(sendLoc, tempResult);
        ExpressionPtr iff;
        if constexpr (Kind == OpAssignKind::And) {
            // Desugar `lhs &&= rhs` to `if (lhs) { lhs = rhs } else { lhs }`
            iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
        } else {
            // OrAsgn: if (lhs) { lhs } else { lhs = rhs }
            iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(elsep), move(body));
        }
        return MK::InsSeq(location, move(stats), move(iff));
    } else {
        // OpAssignKind::Operator
        // Create the read operation: obj.method() or obj[index]
        auto prevValue =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);

        // Apply the operation: prevValue op rhs
        auto newValue = MK::Send1(sendLoc, move(prevValue), op, opLoc, move(rhs));

        // Add the new value to the assignment arguments
        assgnArgs.emplace_back(move(newValue));
        auto numPosAssgnArgs = numPosArgs + 1;

        // Create the assignment operation: obj.method=(newValue) or obj[]=(index, newValue)
        auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                            numPosAssgnArgs, move(assgnArgs), s->flags);

        return MK::InsSeq(location, move(stats), move(res));
    }
}

// Desugar compound assignment when LHS is a safe navigation send (InsSeq from CSend).
// The structure is: { $temp = recv; if $temp == nil then nil else $temp.method }
// We need to modify the else branch to perform the assignment.
template <OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarOpAssignCSend(core::LocOffsets location, ast::InsSeq *insSeq,
                                                    ast::ExpressionPtr rhs, core::NameRef op, core::LocOffsets opLoc) {
    auto ifExpr = ast::cast_tree<ast::If>(insSeq->expr);
    if (!ifExpr) {
        Exception::raise("Unexpected left-hand side of op=: please file an issue");
    }
    auto s = ast::cast_tree<ast::Send>(ifExpr->elsep);
    if (!s) {
        Exception::raise("Unexpected left-hand side of op=: please file an issue");
    }

    auto sendLoc = s->loc;
    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);

    if constexpr (Kind == OpAssignKind::And || Kind == OpAssignKind::Or) {
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(rhs));
        auto cond =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto tempResult = nextUniqueDesugarName(s->fun);
        stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));
        auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                             numPosAssgnArgs, move(assgnArgs), s->flags);
        auto elsep = MK::Local(sendLoc, tempResult);
        auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
        auto wrapped = MK::InsSeq(location, move(stats), move(iff));
        return wrapped;
    } else {
        // OpAssignKind::Operator
        auto prevValue =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto newValue = MK::Send1(sendLoc, move(prevValue), op, opLoc, move(rhs));
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(newValue));

        auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                            numPosAssgnArgs, move(assgnArgs), s->flags);
        auto wrapped = MK::InsSeq(location, move(stats), move(res));
        ifExpr->elsep = move(wrapped);
        return move(insSeq->expr);
    }
}

// Core dispatcher for compound assignment desugaring.
// Routes to the appropriate handler based on the LHS expression type.
template <OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarAnyOpAssign(core::LocOffsets location, ast::ExpressionPtr lhs,
                                                  ast::ExpressionPtr rhs, core::NameRef op, core::LocOffsets opLoc,
                                                  bool isIvarOrCvar) {
    if (auto s = ast::cast_tree<ast::Send>(lhs)) {
        return desugarOpAssignSend<Kind>(location, s, move(rhs), op, opLoc);
    }

    if (isa_reference(lhs)) {
        if constexpr (Kind == OpAssignKind::And || Kind == OpAssignKind::Or) {
            return desugarAndOrReference<Kind>(location, move(lhs), move(rhs), isIvarOrCvar);
        } else {
            return desugarOpReference(location, move(lhs), op, opLoc, move(rhs));
        }
    }

    if (ast::isa_tree<ast::UnresolvedConstantLit>(lhs)) {
        if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
            e.setHeader("Constant reassignment is not supported");
        }
        return MK::EmptyTree();
    }

    if (auto insSeq = ast::cast_tree<ast::InsSeq>(lhs)) {
        return desugarOpAssignCSend<Kind>(location, insSeq, move(rhs), op, opLoc);
    }

    Exception::raise("the LHS has been desugared to something we haven't expected: {}", lhs.toString(ctx));
}

// Maps UnresolvedIdent::Kind to whether it's an ivar or cvar (for T.let special handling)
constexpr bool isIvarOrCvarKind(ast::UnresolvedIdent::Kind kind) {
    return kind == ast::UnresolvedIdent::Kind::Instance || kind == ast::UnresolvedIdent::Kind::Class;
}

// Desugar variable compound assignment nodes.
// Template parameters:
//   - PrismVariableNode: The prism node type (e.g., pm_local_variable_and_write_node)
//   - Kind: The assignment kind (And, Or, Operator)
//   - IdentKind: The kind of identifier (Local, Instance, Class, Global)
template <typename PrismVariableNode, OpAssignKind Kind, ast::UnresolvedIdent::Kind IdentKind>
ast::ExpressionPtr Translator::desugarVariableOpAssign(pm_node_t *untypedNode) {
    auto node = down_cast<PrismVariableNode>(untypedNode);
    auto location = translateLoc(untypedNode->location);
    auto nameLoc = translateLoc(node->name_loc);
    auto name = translateConstantName(node->name);

    auto lhs = ast::make_expression<ast::UnresolvedIdent>(nameLoc, IdentKind, name);
    auto rhs = desugar(node->value);

    constexpr bool isIvarOrCvar = isIvarOrCvarKind(IdentKind);

    if constexpr (Kind == OpAssignKind::Operator) {
        auto opLoc = translateLoc(node->binary_operator_loc);
        auto op = translateConstantName(node->binary_operator);
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), op, opLoc, isIvarOrCvar);
    } else {
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), core::NameRef::noName(),
                                        core::LocOffsets::none(), isIvarOrCvar);
    }
}

// Desugar constant compound assignment nodes (e.g., `CONST &&= val`).
template <typename PrismConstantNode, OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarConstantOpAssign(pm_node_t *untypedNode) {
    auto location = translateLoc(untypedNode->location);

    if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
        e.setHeader("Constant reassignment is not supported");
    }

    auto node = down_cast<PrismConstantNode>(untypedNode);
    auto nameLoc = translateLoc(node->name_loc);
    auto name = translateConstantName(node->name);

    // Handle dynamic constant assignment (inside method def) by using a fake local variable
    if (this->isInMethodDef()) {
        auto lhs = MK::Local(nameLoc, core::Names::dynamicConstAssign());
        auto rhs = desugar(node->value);

        if constexpr (Kind == OpAssignKind::Operator) {
            auto opLoc = translateLoc(node->binary_operator_loc);
            auto op = translateConstantName(node->binary_operator);
            return desugarOpReference(location, move(lhs), op, opLoc, move(rhs));
        } else {
            return desugarAndOrReference<Kind>(location, move(lhs), move(rhs), false);
        }
    }

    auto lhs = MK::UnresolvedConstant(nameLoc, MK::EmptyTree(), name);
    auto rhs = desugar(node->value);

    if constexpr (Kind == OpAssignKind::Operator) {
        auto opLoc = translateLoc(node->binary_operator_loc);
        auto op = translateConstantName(node->binary_operator);
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), op, opLoc, false);
    } else {
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), core::NameRef::noName(),
                                        core::LocOffsets::none(), false);
    }
}

// Desugar constant path compound assignment nodes (e.g., `A::B &&= val`).
template <typename PrismConstantPathNode, OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarConstantPathOpAssign(pm_node_t *untypedNode) {
    auto location = translateLoc(untypedNode->location);

    if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
        e.setHeader("Constant reassignment is not supported");
    }

    auto node = down_cast<PrismConstantPathNode>(untypedNode);
    auto target = node->target;

    auto nameLoc = translateLoc(target->name_loc);
    auto name = translateConstantName(target->name);

    // Translate the constant path to an UnresolvedConstantLit
    ast::ExpressionPtr scope;
    if (target->parent == nullptr) {
        // `::Constant` - top level constant
        scope = MK::Constant(location, core::Symbols::root());
    } else {
        scope = desugar(target->parent);
    }

    auto lhs = MK::UnresolvedConstant(nameLoc, move(scope), name);
    auto rhs = desugar(node->value);

    if constexpr (Kind == OpAssignKind::Operator) {
        auto opLoc = translateLoc(node->binary_operator_loc);
        auto op = translateConstantName(node->binary_operator);
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), op, opLoc, false);
    } else {
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), core::NameRef::noName(),
                                        core::LocOffsets::none(), false);
    }
}

// ----------------------------------------------------------------------------
// Index Op-Assignment Desugaring
// ----------------------------------------------------------------------------

// Desugar index compound assignment nodes (e.g., `arr[i] &&= val`).
template <typename PrismIndexNode, OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarIndexOpAssign(pm_node_t *untypedNode) {
    auto node = down_cast<PrismIndexNode>(untypedNode);
    auto location = translateLoc(untypedNode->location);

    // Handle operator assignment to an indexed expression, like `a[0] += 1`
    auto openingLoc = translateLoc(node->opening_loc);
    auto lBracketLoc = core::LocOffsets{openingLoc.beginLoc, openingLoc.endLoc - 1};

    auto receiverExpr = desugar(node->receiver);
    auto argsStore = desugarArguments<ast::Send::ARGS_store>(node->arguments, up_cast(node->block));

    // The LHS location includes the receiver and the `[]`, but not the `=` or rhs.
    auto lhsLoc = translateLoc(node->receiver->location.start, node->closing_loc.end);

    // Create the LHS Send expression: recv[]
    auto lhs = MK::Send(lhsLoc, move(receiverExpr), core::Names::squareBrackets(), lBracketLoc, argsStore.size(),
                        move(argsStore));
    auto rhs = desugar(node->value);

    if constexpr (Kind == OpAssignKind::Operator) {
        auto opLoc = translateLoc(node->binary_operator_loc);
        auto op = translateConstantName(node->binary_operator);
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), op, opLoc, false);
    } else {
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), core::NameRef::noName(),
                                        core::LocOffsets::none(), false);
    }
}

// ----------------------------------------------------------------------------
// Send (Call) Op-Assignment Desugaring
// ----------------------------------------------------------------------------

// Desugar send compound assignment nodes (e.g., `obj.method &&= val`, `obj.method += val`).
// Handles both regular sends and safe navigation sends (CSend).
template <typename PrismSendNode, OpAssignKind Kind>
ast::ExpressionPtr Translator::desugarSendOpAssign(pm_node_t *untypedNode) {
    auto node = down_cast<PrismSendNode>(untypedNode);
    auto location = translateLoc(untypedNode->location);
    auto name = translateConstantName(node->read_name);
    auto receiverExpr = desugar(node->receiver);
    auto messageLoc = translateLoc(node->message_loc);

    // The lhs's location spans from the start of the receiver to the end of the message
    auto lhsLoc = core::LocOffsets{location.beginPos(), messageLoc.endPos()};

    if (PM_NODE_FLAG_P(untypedNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
        // Handle safe navigation: a&.b += 1
        // Creates pattern: { $temp = a; if $temp == nil then nil else $temp.b += 1 }
        auto tempRecv = nextUniqueDesugarName(core::Names::assignTemp());
        auto recvLoc = receiverExpr.loc();
        auto zeroLengthLoc = location.copyWithZeroLength();
        auto zeroLengthRecvLoc = recvLoc.copyWithZeroLength();

        // The `&` in `a&.b = 1`
        constexpr auto len = "&"sv.size();
        auto ampersandLoc = translateLoc(node->call_operator_loc.start, node->call_operator_loc.start + len);

        auto tempAssign = MK::Assign(zeroLengthRecvLoc, tempRecv, move(receiverExpr));
        auto cond = MK::Send1(zeroLengthLoc, MK::Constant(zeroLengthRecvLoc, core::Symbols::NilClass()),
                              core::Names::tripleEq(), zeroLengthRecvLoc, MK::Local(zeroLengthRecvLoc, tempRecv));

        // Create the inner send: $temp.b
        auto innerSend =
            MK::Send(location, MK::Local(zeroLengthRecvLoc, tempRecv), name, messageLoc, 0, ast::Send::ARGS_store{});

        auto rhs = desugar(node->value);

        ast::ExpressionPtr assignmentExpr;
        if constexpr (Kind == OpAssignKind::Operator) {
            auto opLoc = translateLoc(node->binary_operator_loc);
            auto op = translateConstantName(node->binary_operator);
            assignmentExpr = desugarAnyOpAssign<Kind>(location, move(innerSend), move(rhs), op, opLoc, false);
        } else {
            assignmentExpr = desugarAnyOpAssign<Kind>(location, move(innerSend), move(rhs), core::NameRef::noName(),
                                                      core::LocOffsets::none(), false);
        }

        auto nilValue =
            MK::Send1(recvLoc.copyEndWithZeroLength(), MK::Magic(zeroLengthLoc), core::Names::nilForSafeNavigation(),
                      zeroLengthLoc, MK::Local(ampersandLoc, tempRecv));
        auto ifExpr = MK::If(zeroLengthLoc, move(cond), move(nilValue), move(assignmentExpr));
        return MK::InsSeq1(location, move(tempAssign), move(ifExpr));
    }

    // Regular send: a.b += 1
    ast::Send::Flags flags;
    flags.isPrivateOk = PM_NODE_FLAG_P(untypedNode, PM_CALL_NODE_FLAGS_IGNORE_VISIBILITY);

    auto lhs = MK::Send(lhsLoc, move(receiverExpr), name, messageLoc, 0, ast::Send::ARGS_store{}, flags);
    auto rhs = desugar(node->value);

    if constexpr (Kind == OpAssignKind::Operator) {
        auto opLoc = translateLoc(node->binary_operator_loc);
        auto op = translateConstantName(node->binary_operator);
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), op, opLoc, false);
    } else {
        return desugarAnyOpAssign<Kind>(location, move(lhs), move(rhs), core::NameRef::noName(),
                                        core::LocOffsets::none(), false);
    }
}

// ----------------------------------------------------------------------------
// Regular Assignment Desugaring
// ----------------------------------------------------------------------------

// Desugar regular (non-compound) assignment nodes.
// Handles: local, instance, class, global variable assignments, and constant assignments.
// Template parameters:
//   - PrismAssignmentNode: The prism node type (e.g., pm_local_variable_write_node)
//   - IdentKind: The kind of identifier (Local, Instance, Class, Global), or void for constants
template <typename PrismAssignmentNode, ast::UnresolvedIdent::Kind IdentKind>
ast::ExpressionPtr Translator::desugarAssignment(pm_node_t *untypedNode) {
    auto node = down_cast<PrismAssignmentNode>(untypedNode);
    auto location = translateLoc(untypedNode->location);
    auto rhs = desugar(node->value);

    ast::ExpressionPtr lhs;
    if constexpr (is_same_v<PrismAssignmentNode, pm_constant_write_node>) {
        // Handle regular assignment to a "plain" constant, like `A = 1`
        auto nameLoc = translateLoc(node->name_loc);

        // Check for dynamic constant assignment (constant assigned inside a method)
        if (this->isInMethodDef()) {
            // Substitute a fake local variable assignment so parsing can continue
            lhs = MK::Local(nameLoc, core::Names::dynamicConstAssign());
        } else {
            auto name = translateConstantName(node->name);
            auto constantName = ctx.state.enterNameConstant(name);
            lhs = MK::UnresolvedConstant(nameLoc, MK::EmptyTree(), constantName);
        }
    } else if constexpr (is_same_v<PrismAssignmentNode, pm_constant_path_write_node>) {
        // Handle regular assignment to a constant path, like `A::B::C = 1` or `::C = 1`
        auto target = node->target;
        auto pathLoc = translateLoc(target->base.location);

        // Check for dynamic constant assignment (constant assigned inside a method)
        if (this->isInMethodDef()) {
            lhs = MK::Local(pathLoc, core::Names::dynamicConstAssign());
        } else {
            ast::ExpressionPtr scope;
            if (target->parent == nullptr) {
                // `::Constant` - top level constant
                auto delimiterLoc = translateLoc(target->delimiter_loc);
                scope = MK::Constant(delimiterLoc, core::Symbols::root());
            } else {
                scope = desugar(target->parent);
            }
            auto name = translateConstantName(target->name);
            auto constantName = ctx.state.enterNameConstant(name);
            lhs = MK::UnresolvedConstant(pathLoc, move(scope), constantName);
        }
    } else {
        // Handle regular assignment to local, instance, class, or global variable
        auto name = translateConstantName(node->name);
        auto nameLoc = translateLoc(node->name_loc);
        lhs = ast::make_expression<ast::UnresolvedIdent>(nameLoc, IdentKind, name);
    }

    return MK::Assign(location, move(lhs), move(rhs));
}

pair<core::LocOffsets, core::LocOffsets>
Translator::computeMethodCallLoc(core::LocOffsets initialLoc, pm_node_t *receiver, absl::Span<pm_node_t *> prismArgs,
                                 pm_location_t closingLoc, const Translator::DesugaredBlockArgument &block) {
    auto result = initialLoc;

    if (receiver) {
        result = translateLoc(receiver->location).join(result);
    }

    // Extend the location to include the closing `)`/`]` of the arguments, if any, but only for `pm_call_node`s.
    // Not for `pm_lambda_node` though, because its `closing_loc` is the closing `}` or `end` of the block.
    if (closingLoc.start && closingLoc.end) {
        result = result.join(translateLoc(closingLoc));
    }

    if (!prismArgs.empty()) { // Extend to last argument's location, if any.
        // For index expressions, the closing_loc can come before the last
        // argument's location:
        //     a[1, 2] = 3
        //           ^     closing loc
        //               ^ last arg loc
        result = result.join(translateLoc(prismArgs.back()->location));
    }

    core::LocOffsets blockLoc;
    if (block.hasBlockPass()) {
        result = result.join(block.blockPassExpr.loc());

        blockLoc = block.blockPassExpr.loc();
    } else if (block.hasLiteralBlock()) {
        blockLoc = block.literalBlockExpr.loc();
    }

    return {result, blockLoc};
}

ast::ExpressionPtr Translator::desugarNullable(pm_node_t *node) {
    if (node == nullptr) {
        return MK::EmptyTree();
    }

    return desugar(node);
}

std::unique_ptr<parser::Node> Translator::translate_TODO(pm_node_t *node) {
    auto expr = desugar(node);
    auto loc = expr.loc();

    if (!expr.loc().exists()) {
        loc = translateLoc(node->location);
    }

    return make_unique<ExprOnly>(move(expr), loc);
}

ast::ExpressionPtr Translator::desugar(pm_node_t *node) {
    if (node == nullptr)
        return nullptr;

    auto location = translateLoc(node->location);

    switch (PM_NODE_TYPE(node)) {
        case PM_ALIAS_GLOBAL_VARIABLE_NODE: { // The `alias` keyword used for global vars, like `alias $new $old`
            auto aliasGlobalVariableNode = down_cast<pm_alias_global_variable_node>(node);

            auto toExpr = desugar(aliasGlobalVariableNode->new_name);
            auto fromExpr = desugar(aliasGlobalVariableNode->old_name);

            // Desugar `alias $new $old` to `self.alias_method($new, $old)`
            return MK::Send2(location, MK::Self(location), core::Names::aliasMethod(), location.copyWithZeroLength(),
                             std::move(toExpr), std::move(fromExpr));
        }
        case PM_ALIAS_METHOD_NODE: { // The `alias` keyword, like `alias new_method old_method`
            auto aliasMethodNode = down_cast<pm_alias_method_node>(node);

            auto toExpr = desugar(aliasMethodNode->new_name);
            auto fromExpr = desugar(aliasMethodNode->old_name);

            // Desugar methods: `alias new old` to `self.alias_method(new, old)`
            return MK::Send2(location, MK::Self(location), core::Names::aliasMethod(), location.copyWithZeroLength(),
                             std::move(toExpr), std::move(fromExpr));
        }
        case PM_AND_NODE: { // operator `&&` and `and`
            auto andNode = down_cast<pm_and_node>(node);

            auto lhs = desugarNullable(andNode->left);
            auto rhs = desugarNullable(andNode->right);

            if (preserveConcreteSyntax) {
                auto andAndLoc = core::LocOffsets{lhs.loc().endPos(), rhs.loc().beginPos()};
                return MK::Send2(location, MK::Magic(location.copyWithZeroLength()), core::Names::andAnd(), andAndLoc,
                                 move(lhs), move(rhs));
            }

            if (isa_reference(lhs)) {
                auto cond = MK::cpRef(lhs);
                return MK::If(location, move(cond), move(rhs), move(lhs));
            }

            // For non-reference expressions, create a temporary variable so we don't evaluate the LHS twice.
            // E.g. `x = 1 && 2` becomes `x = (temp = 1; temp ? temp : 2)`
            core::NameRef tempLocalName = nextUniqueDesugarName(core::Names::andAnd());

            // Capture locations before any moves
            auto lhsLoc = lhs.loc();
            auto rhsLoc = rhs.loc();

            bool checkAndAnd = ast::isa_tree<ast::Send>(lhs) && ast::isa_tree<ast::Send>(rhs);
            ExpressionPtr thenp;
            if (checkAndAnd) {
                auto lhsSend = ast::cast_tree<ast::Send>(lhs);
                auto rhsSend = ast::cast_tree<ast::Send>(rhs);
                auto lhsSource = ctx.locAt(lhsSend->loc).source(ctx);
                auto rhsRecvSource = ctx.locAt(rhsSend->recv.loc()).source(ctx);
                if (lhsSource.has_value() && lhsSource == rhsRecvSource) {
                    // Create the magic <check-and-and> call

                    // Modify rhsSend in place to create the magic <check-and-and> call
                    // This matches the legacy desugaring approach
                    rhsSend->insertPosArg(0, move(rhsSend->recv));
                    rhsSend->insertPosArg(1, MK::Symbol(rhsSend->funLoc.copyWithZeroLength(), rhsSend->fun));
                    rhsSend->insertPosArg(2, MK::Local(location.copyWithZeroLength(), tempLocalName));
                    rhsSend->recv = MK::Magic(location.copyWithZeroLength());
                    rhsSend->fun = core::Names::checkAndAnd();
                    thenp = move(rhs);
                } else {
                    thenp = move(rhs);
                }
            } else {
                thenp = move(rhs);
            }
            auto condLoc =
                lhsLoc.exists() && rhsLoc.exists() ? core::LocOffsets{lhsLoc.endPos(), rhsLoc.beginPos()} : lhsLoc;
            auto temp = MK::Assign(location, tempLocalName, move(lhs));
            auto elsep = MK::Local(lhsLoc, tempLocalName);
            auto cond = MK::Local(condLoc, tempLocalName);
            auto if_ = MK::If(location, move(cond), move(thenp), move(elsep));
            return MK::InsSeq1(location, move(temp), move(if_));
        }
        case PM_ARGUMENTS_NODE: { // A list of arguments in one of several places:
            // 1. The arguments to a method call, e.g the `1, 2, 3` in `f(1, 2, 3)`.
            // 2. The value(s) returned from a return statement, e.g. the `1, 2, 3` in `return 1, 2, 3`.
            // 3. The arguments to a `yield` call, e.g. the `1, 2, 3` in `yield 1, 2, 3`.
            unreachable("PM_ARGUMENTS_NODE is handled separately in `Translator::translateArguments()`.");
        }
        case PM_ARRAY_NODE: { // An array literal, e.g. `[1, 2, 3]`
            auto arrayNode = down_cast<pm_array_node>(node);

            auto prismElements = absl::MakeSpan(arrayNode->elements.nodes, arrayNode->elements.size);
            auto elements = nodeListToStore<ast::Array::ENTRY_store>(arrayNode->elements);

            return desugarArray(location, prismElements, move(elements));
        }
        case PM_ASSOC_NODE: { // A key-value pair in a Hash literal, e.g. the `a: 1` in `{ a: 1 }
            unreachable("PM_ASSOC_NODE is handled specially in every context where it might appear.");
        }
        case PM_ASSOC_SPLAT_NODE: { // A Hash splat, e.g. `**h` in `f(a: 1, **h)` and `{ k: v, **h }`
            unreachable("PM_ASSOC_SPLAT_NODE is handled separately in `desugarKeyValuePairs()` and "
                        "`PM_HASH_PATTERN_NODE`, because its translation depends on whether its used in a "
                        "Hash literal, Hash pattern, or method call.");
        }
        case PM_BACK_REFERENCE_READ_NODE: { // One of the special global variables for accessing info about the previous
                                            // Regexp match, such as `$&`, `$`, `$'`, and `$+`.
            auto backReferenceReadNode = down_cast<pm_back_reference_read_node>(node);
            auto name = translateConstantName(backReferenceReadNode->name);

            // Desugar `$&` to `<Magic>.<regex-backref>(:&)`
            auto recv = MK::Magic(location);
            auto arg = MK::Symbol(location, name);
            auto locZeroLen = location.copyWithZeroLength();
            return MK::Send1(location, move(recv), core::Names::regexBackref(), locZeroLen, move(arg));
        }
        case PM_BEGIN_NODE: { // A `begin ... end` block
            auto beginNode = down_cast<pm_begin_node>(node);
            return desugarBegin(beginNode);
        }
        case PM_BLOCK_ARGUMENT_NODE: { // A block arg passed into a method call, e.g. the `&b` in `a.map(&b)`
            unreachable("PM_BLOCK_ARGUMENT_NODE is handled specially in `desugarArguments()`, see it for details.");
        }
        case PM_BLOCK_NODE: { // An explicit block passed to a method call, i.e. `{ ... }` or `do ... end`
            unreachable("PM_BLOCK_NODE has special handling in PM_CALL_NODE, see it for details.");
        }
        case PM_BLOCK_LOCAL_VARIABLE_NODE: { // A named block local variable, like `baz` in `|bar; baz|`
            auto blockLocalNode = down_cast<pm_block_local_variable_node>(node);
            auto sorbetName = translateConstantName(blockLocalNode->name);
            return MK::ShadowArg(location, MK::Local(location, sorbetName));
        }
        case PM_BLOCK_PARAMETER_NODE: { // A block parameter declared at the top of a method, e.g. `def m(&block)`
            unreachable("PM_BLOCK_PARAMETER_NODE is handled separately in `Translator::desugarParametersNode()`.");
        }
        case PM_BLOCK_PARAMETERS_NODE: { // The parameters declared at the top of a PM_BLOCK_NODE
            unreachable("PM_BLOCK_PARAMETERS_NODE is handled separately in `PM_CALL_NODE`.");
        }
        case PM_BREAK_NODE: { // A `break` statement, e.g. `break`, `break 1, 2, 3`
            auto breakNode = down_cast<pm_break_node>(node);
            auto arguments = desugarBreakNextReturn(breakNode->arguments);
            return MK::Break(location, move(arguments));
        }
        case PM_CALL_AND_WRITE_NODE: { // And-assignment to a method call, e.g. `a.b &&= false`
            return desugarSendOpAssign<pm_call_and_write_node, OpAssignKind::And>(node);
        }
        case PM_CALL_NODE: { // A method call like `a.b()` or `a&.b()`
            auto callNode = down_cast<pm_call_node>(node);

            auto constantNameString = parser.resolveConstant(callNode->name);
            auto methodName = ctx.state.enterNameUTF8(constantNameString);

            auto receiverNode = callNode->receiver;

            ast::ExpressionPtr receiver;
            if (receiverNode == nullptr) { // Convert `foo()` to `self.foo()`
                // 0-sized Loc, since `self.` doesn't appear in the original file.
                receiver = MK::Self(location.copyWithZeroLength());
            } else {
                receiver = desugar(receiverNode);
            }

            // Unsupported nodes are desugared to an empty tree.
            // Treat them as if they were `self` to match `Desugar.cc`.
            // TODO: Clean up after direct desugaring is complete.
            // https://github.com/Shopify/sorbet/issues/671
            bool isPrivateOk;
            if (ast::isa_tree<ast::EmptyTree>(receiver)) {
                receiver = MK::Self(location.copyWithZeroLength());
                isPrivateOk = true;
            } else {
                isPrivateOk = PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_IGNORE_VISIBILITY);
            }

            if (methodName == core::Names::blockGiven_p()) {
                categoryCounterInc("Prism fallback", "block_given?");
                throw PrismFallback{};
            }

            // When the message is empty, like `foo.()`, the message location is the
            // same as the call operator location
            core::LocOffsets methodNameLoc;
            if (callNode->message_loc.start == nullptr && callNode->message_loc.end == nullptr) {
                methodNameLoc = translateLoc(callNode->call_operator_loc);
            } else {
                methodNameLoc = translateLoc(callNode->message_loc);
            }

            auto block = desugarBlock(callNode->block, callNode->arguments, callNode->base.location);

            if (PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                categoryCounterInc("Prism fallback", "PM_CALL_NODE_FLAGS_SAFE_NAVIGATION");
                throw PrismFallback{};
            }

            return desugarMethodCall(move(receiver), methodName, methodNameLoc, callNode->arguments,
                                     callNode->closing_loc, move(block), location, isPrivateOk);
        }
        case PM_CALL_OPERATOR_WRITE_NODE: { // Compound assignment to a method call, e.g. `a.b += 1`
            return desugarSendOpAssign<pm_call_operator_write_node, OpAssignKind::Operator>(node);
        }
        case PM_CALL_OR_WRITE_NODE: { // Or-assignment to a method call, e.g. `a.b ||= true`
            return desugarSendOpAssign<pm_call_or_write_node, OpAssignKind::Or>(node);
        }
        case PM_CALL_TARGET_NODE: { // Target of an indirect write to the result of a method call
            // ... like `self.target1, self.target2 = 1, 2`, `rescue => self.target`, etc.
            categoryCounterInc("Prism fallback", "PM_CALL_TARGET_NODE");
            throw PrismFallback{};
        }
        case PM_CASE_MATCH_NODE: { // A pattern-matching `case` statement that only uses `in` (and not `when`)
            auto caseMatchNode = down_cast<pm_case_match_node>(node);

            auto inNodes = absl::MakeSpan(caseMatchNode->conditions.nodes, caseMatchNode->conditions.size);
            auto elseClause = desugarNullable(up_cast(caseMatchNode->else_clause));

            // Build an if ladder similar to CASE_NODE

            // Start with the else clause as the final else
            ExpressionPtr resultExpr = move(elseClause);

            // Build the if ladder backwards from the last "in" to the first
            for (auto it = inNodes.rbegin(); it != inNodes.rend(); ++it) {
                ENFORCE(PM_NODE_TYPE_P(*it, PM_IN_NODE));
                auto inPattern = down_cast<pm_in_node>(*it);

                // Get the actual pattern location (unwrapping if/unless guards)
                pm_location_t patternLoc = inPattern->base.location;
                pm_node_t *actualPattern = inPattern->pattern;
                if (actualPattern != nullptr) {
                    if (PM_NODE_TYPE_P(actualPattern, PM_IF_NODE)) {
                        auto ifNode = down_cast<pm_if_node>(actualPattern);
                        if (ifNode->statements != nullptr && ifNode->statements->body.size > 0) {
                            patternLoc = ifNode->statements->body.nodes[0]->location;
                        }
                    } else if (PM_NODE_TYPE_P(actualPattern, PM_UNLESS_NODE)) {
                        auto unlessNode = down_cast<pm_unless_node>(actualPattern);
                        if (unlessNode->statements != nullptr && unlessNode->statements->body.size > 0) {
                            patternLoc = unlessNode->statements->body.nodes[0]->location;
                        }
                    } else {
                        patternLoc = actualPattern->location;
                    }
                }

                // Use RaiseUnimplemented as the condition (like desugarOnelinePattern)
                auto matchExpr = MK::RaiseUnimplemented(translateLoc(patternLoc));

                // The body is the statements from the "in" clause
                auto thenExpr = desugarStatements(inPattern->statements);

                // Collect pattern variable assignments from the pattern
                ast::InsSeq::STATS_store vars;
                collectPatternMatchingVarsPrism(vars, inPattern->pattern);
                if (!vars.empty()) {
                    auto loc = translateLoc(patternLoc);
                    thenExpr = MK::InsSeq(loc, move(vars), move(thenExpr));
                }

                resultExpr =
                    MK::If(translateLoc(inPattern->base.location), move(matchExpr), move(thenExpr), move(resultExpr));
            }

            // Wrap in an InsSeq with the predicate assignment (if there is a predicate)
            auto predicate = desugar(caseMatchNode->predicate);
            auto tempName = nextUniqueDesugarName(core::Names::assignTemp());
            auto assignExpr = MK::Assign(predicate.loc(), tempName, move(predicate));

            return MK::InsSeq1(location, move(assignExpr), move(resultExpr));
        }
        case PM_CASE_NODE: { // A classic `case` statement that only uses `when` (and not pattern matching with `in`)
            auto caseNode = down_cast<pm_case_node>(node);

            auto predicate = desugarNullable(caseNode->predicate);

            auto prismWhenNodes = absl::MakeSpan(caseNode->conditions.nodes, caseNode->conditions.size);

            // Count the total number of patterns across all when clauses
            size_t totalPatterns = 0;
            for (auto *whenNodePtr : prismWhenNodes) {
                auto *whenNode = down_cast<pm_when_node>(whenNodePtr);
                totalPatterns += whenNode->conditions.size;
            }

            auto elseClause = desugarNullable(up_cast(caseNode->else_clause));

            if (preserveConcreteSyntax) {
                auto locZeroLen = location.copyWithZeroLength();

                ast::Send::ARGS_store args;
                args.reserve(2 + prismWhenNodes.size() +
                             totalPatterns); // +2 is for the predicate and the patterns count
                args.emplace_back(move(predicate));
                args.emplace_back(MK::Int(locZeroLen, totalPatterns));

                // Extract pattern expressions directly from Prism nodes
                for (auto *prismWhenPtr : prismWhenNodes) {
                    auto *prismWhen = down_cast<pm_when_node>(prismWhenPtr);
                    auto prismPatterns = absl::MakeSpan(prismWhen->conditions.nodes, prismWhen->conditions.size);

                    for (auto *prismPattern : prismPatterns) {
                        auto pattern = desugarNullable(prismPattern);
                        args.emplace_back(move(pattern));
                    }
                }

                // Extract body expressions directly from Prism nodes
                for (auto *prismWhenPtr : prismWhenNodes) {
                    auto *prismWhen = down_cast<pm_when_node>(prismWhenPtr);
                    auto body = desugarStatements(prismWhen->statements);
                    args.emplace_back(move(body));
                }

                args.emplace_back(move(elseClause));

                // Desugar to `::Magic.caseWhen(predicate, num_patterns, patterns..., bodies..., else)`
                return MK::Send(location, MK::Magic(locZeroLen), core::Names::caseWhen(), locZeroLen, args.size(),
                                move(args));
            }

            core::NameRef tempName;
            core::LocOffsets predicateLoc;
            bool hasPredicate = (predicate != nullptr);

            if (hasPredicate) {
                predicateLoc = predicate.loc();
                tempName = nextUniqueDesugarName(core::Names::assignTemp());
            } else {
                tempName = core::NameRef::noName();
            }

            // The if/else ladder for the entire case statement, starting with the else clause as the final `else` when
            // building backwards
            ExpressionPtr resultExpr = move(elseClause);

            // Iterate over Prism when nodes in reverse to build the if/else ladder backwards
            for (auto it = prismWhenNodes.rbegin(); it != prismWhenNodes.rend(); ++it) {
                auto *prismWhen = down_cast<pm_when_node>(*it);
                auto whenLoc = translateLoc(prismWhen->base.location);
                auto prismPatterns = absl::MakeSpan(prismWhen->conditions.nodes, prismWhen->conditions.size);

                ExpressionPtr patternsResult; // the if/else ladder for this when clause's patterns
                for (auto *prismPattern : prismPatterns) {
                    auto pattern = desugarNullable(prismPattern);
                    auto patternLoc = pattern.loc();

                    ExpressionPtr testExpr;
                    if (PM_NODE_TYPE_P(prismPattern, PM_SPLAT_NODE)) {
                        // splat pattern in when clause, predicate is required, `case a when *others`
                        ENFORCE(hasPredicate, "splats need something to test against");
                        auto local = MK::Local(predicateLoc, tempName);
                        // Desugar `case x when *patterns` to `::Magic.<check-match-array>(x, patterns)`,
                        // which behaves like `patterns.any?(x)`
                        testExpr = MK::Send2(patternLoc, MK::Magic(location), core::Names::checkMatchArray(),
                                             patternLoc.copyWithZeroLength(), move(local), move(pattern));
                    } else if (hasPredicate) {
                        // regular pattern when case predicate is present, `case a when 1`
                        auto local = MK::Local(predicateLoc, tempName);
                        // Desugar `case x when 1` to `1 === x`
                        testExpr = MK::Send1(patternLoc, move(pattern), core::Names::tripleEq(),
                                             patternLoc.copyWithZeroLength(), move(local));
                    } else {
                        // regular pattern when case predicate is not present, `case when 1 then "one" end`
                        // case # no predicate present
                        // when 1
                        //   "one"
                        // end
                        testExpr = move(pattern);
                    }

                    if (patternsResult == nullptr) {
                        patternsResult = move(testExpr);
                    } else {
                        auto trueExpr = MK::True(testExpr.loc());
                        patternsResult = MK::If(testExpr.loc(), move(testExpr), move(trueExpr), move(patternsResult));
                    }
                }

                auto then = desugarStatements(prismWhen->statements);
                resultExpr = MK::If(whenLoc, move(patternsResult), move(then), move(resultExpr));
            }

            if (hasPredicate) {
                auto assignExpr = MK::Assign(predicateLoc, tempName, move(predicate));
                resultExpr = MK::InsSeq1(location, move(assignExpr), move(resultExpr));
            }

            return resultExpr;
        }
        case PM_CLASS_NODE: { // Class declarations, not including singleton class declarations (`class <<`)
            auto classNode = down_cast<pm_class_node>(node);

            auto name = desugar(classNode->constant_path);
            auto declLoc = translateLoc(classNode->class_keyword_loc).join(name.loc());

            ast::ClassDef::ANCESTORS_store ancestors;
            if (classNode->superclass) {
                auto superclass = desugar(classNode->superclass);
                declLoc = declLoc.join(superclass.loc());
                ancestors.emplace_back(move(superclass));
            } else {
                ancestors.emplace_back(MK::Constant(location, core::Symbols::todo()));
            }

            auto body = this->enterClassContext(enclosingBlockParamLoc, enclosingBlockParamName)
                            .desugarClassOrModule(classNode->body);

            return MK::Class(location, declLoc, move(name), move(ancestors), move(body));
        }
        case PM_CLASS_VARIABLE_AND_WRITE_NODE: { // And-assignment to a class variable, e.g. `@@a &&= 1`
            return desugarVariableOpAssign<pm_class_variable_and_write_node, OpAssignKind::And,
                                           ast::UnresolvedIdent::Kind::Class>(node);
        }
        case PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a class variable, e.g. `@@a += 1`
            return desugarVariableOpAssign<pm_class_variable_operator_write_node, OpAssignKind::Operator,
                                           ast::UnresolvedIdent::Kind::Class>(node);
        }
        case PM_CLASS_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a class variable, e.g. `@@a ||= 1`
            return desugarVariableOpAssign<pm_class_variable_or_write_node, OpAssignKind::Or,
                                           ast::UnresolvedIdent::Kind::Class>(node);
        }
        case PM_CLASS_VARIABLE_READ_NODE: { // A class variable, like `@@a`
            auto classVarNode = down_cast<pm_class_variable_read_node>(node);
            auto name = translateConstantName(classVarNode->name);
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Class, name);
        }
        case PM_CLASS_VARIABLE_TARGET_NODE: { // Target of an indirect write to a class variable
            // ... like `@@target1, @@target2 = 1, 2`, `rescue => @@target`, etc.
            auto classVariableTargetNode = down_cast<pm_class_variable_target_node>(node);
            auto name = translateConstantName(classVariableTargetNode->name);
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Class, name);
        }
        case PM_CLASS_VARIABLE_WRITE_NODE: { // Regular assignment to a class variable, e.g. `@@a = 1`
            return desugarAssignment<pm_class_variable_write_node, ast::UnresolvedIdent::Kind::Class>(node);
        }
        case PM_CONSTANT_PATH_AND_WRITE_NODE: { // And-assignment to a constant path, e.g. `A::B &&= false`
            return desugarConstantPathOpAssign<pm_constant_path_and_write_node, OpAssignKind::And>(node);
        }
        case PM_CONSTANT_PATH_NODE: { // Part of a constant path, like the `A::B` in `A::B::C`.
            // See`PM_CONSTANT_READ_NODE`, which handles the `::C` part
            return translateConst<pm_constant_path_node>(node);
        }
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE: { // Compound assignment to a constant path, e.g. `A::B += 1`
            return desugarConstantPathOpAssign<pm_constant_path_operator_write_node, OpAssignKind::Operator>(node);
        }
        case PM_CONSTANT_PATH_OR_WRITE_NODE: { // Or-assignment to a constant path, e.g. `A::B ||= true`
            return desugarConstantPathOpAssign<pm_constant_path_or_write_node, OpAssignKind::Or>(node);
        }
        case PM_CONSTANT_PATH_TARGET_NODE: { // Target of an indirect write to a constant path
            // ... like `A::TARGET1, A::TARGET2 = 1, 2`, `rescue => A::TARGET`, etc.
            return translateConst<pm_constant_path_target_node>(node);
        }
        case PM_CONSTANT_PATH_WRITE_NODE: { // Regular assignment to a constant path, e.g. `A::B = 1`
            return desugarAssignment<pm_constant_path_write_node>(node);
        }
        case PM_CONSTANT_TARGET_NODE: { // Target of an indirect write to a constant
            // ... like `TARGET1, TARGET2 = 1, 2`, `rescue => TARGET`, etc.
            return translateConst<pm_constant_target_node>(node);
        }
        case PM_CONSTANT_AND_WRITE_NODE: { // And-assignment to a constant, e.g. `C &&= false`
            return desugarConstantOpAssign<pm_constant_and_write_node, OpAssignKind::And>(node);
        }
        case PM_CONSTANT_OPERATOR_WRITE_NODE: { // Compound assignment to a constant, e.g. `C += 1`
            return desugarConstantOpAssign<pm_constant_operator_write_node, OpAssignKind::Operator>(node);
        }
        case PM_CONSTANT_OR_WRITE_NODE: { // Or-assignment to a constant, e.g. `C ||= true`
            return desugarConstantOpAssign<pm_constant_or_write_node, OpAssignKind::Or>(node);
        }
        case PM_CONSTANT_READ_NODE: { // A single, unnested, non-fully qualified constant like `Foo`
            return translateConst<pm_constant_read_node>(node);
        }
        case PM_CONSTANT_WRITE_NODE: { // Regular assignment to a constant, e.g. `Foo = 1`
            return desugarAssignment<pm_constant_write_node>(node);
        }
        case PM_DEF_NODE: { // Method definitions, like `def m; ...; end` and `def m = 123`
            auto defNode = down_cast<pm_def_node>(node);
            auto declLoc = translateLoc(defNode->def_keyword_loc);
            declLoc = declLoc.join(translateLoc(defNode->name_loc));

            auto rparenLoc = defNode->rparen_loc;

            // If there is a `)` in the method definition, include it in the location span.
            if (rparenLoc.start != nullptr && rparenLoc.end != nullptr) {
                declLoc = declLoc.join(translateLoc(defNode->rparen_loc));
            }

            auto receiver = desugarNullable(defNode->receiver); // The singleton receiver, like `self` in `self.foo()`
            auto name = translateConstantName(defNode->name);

            auto isSingletonMethod = !ast::isa_tree<ast::EmptyTree>(receiver);

            core::LocOffsets enclosingBlockParamLoc;
            core::NameRef enclosingBlockParamName;
            ast::MethodDef::PARAMS_store paramsStore;
            ast::InsSeq::STATS_store statsStore;
            if (defNode->parameters != nullptr) {
                // The Params' location shouldn't include the `( )`, if any.
                //    def foo(a, b)
                //            ^^^^
                auto startLoc = defNode->lparen_loc.start == nullptr ? defNode->parameters->base.location.start
                                                                     : defNode->lparen_loc.start;
                auto endLoc = defNode->rparen_loc.end == nullptr ? defNode->parameters->base.location.end
                                                                 : defNode->rparen_loc.end;
                auto loc = translateLoc(pm_location_t{startLoc, endLoc});
                declLoc = declLoc.join(loc);

                std::tie(paramsStore, statsStore, enclosingBlockParamLoc, enclosingBlockParamName) =
                    desugarParametersNode(defNode->parameters, loc);
            } else {
                if (rparenLoc.start != nullptr) {
                    // The definition has no parameters but still has parentheses, e.g. `def foo(); end`
                    // In this case, Sorbet's legacy parser will still hold an empty Args node
                    auto loc = translateLoc(defNode->lparen_loc.start, defNode->rparen_loc.end);

                    std::tie(paramsStore, statsStore, enclosingBlockParamLoc, enclosingBlockParamName) =
                        desugarParametersNode(nullptr, loc);
                }

                // Desugaring a method def like `def foo()` should behave like `def foo(&<blk>)`,
                // so we set a synthetic name here for `yield` to use.
                enclosingBlockParamName = core::Names::blkArg();
            }

            Translator methodContext =
                this->enterMethodDef(isSingletonMethod, declLoc, name, enclosingBlockParamLoc, enclosingBlockParamName);

            ast::ExpressionPtr body;
            if (defNode->body != nullptr) {
                if (PM_NODE_TYPE_P(defNode->body, PM_BEGIN_NODE)) {
                    // Prism uses a PM_BEGIN_NODE to model the body of a method that has a top level rescue/ensure, e.g.
                    //
                    //     def method_with_top_level_rescue
                    //       "body"
                    //     rescue
                    //       "fallback"
                    //     end
                    //
                    // desugarBegin handles this directly, returning the desugared expression.
                    auto beginNode = down_cast<pm_begin_node>(defNode->body);
                    body = methodContext.desugarBegin(beginNode);
                } else {
                    // Side effect: If method has no explicit block parameter and contains a `yield`, translating it
                    // will change the `methodContext.enclosingBlockParamName` from `<blk>` to `<implicit yield>`.
                    // This will modify the local `enclosingBlockParamName` that it references.
                    // The `methodContext.enclosingBlockParamLoc`/`enclosingBlockParamLoc` are similarly modified.
                    body = methodContext.desugarNullable(defNode->body);
                }
            } else {
                body = MK::EmptyTree();
            }

            if (!statsStore.empty()) {
                auto bodyLoc = body.loc();
                if (!bodyLoc.exists()) {
                    bodyLoc = location;
                }
                body = MK::InsSeq(bodyLoc, move(statsStore), move(body));
            }

            // Add an implicit block parameter, if no explicit one was declared in the parameter list.
            if (paramsStore.empty() || !ast::isa_tree<ast::BlockParam>(paramsStore.back())) {
                auto blkLoc = core::LocOffsets::none();
                paramsStore.emplace_back(MK::BlockParam(
                    blkLoc, MK::Local(methodContext.enclosingBlockParamLoc, methodContext.enclosingBlockParamName)));
            }

            auto methodExpr = MK::Method(location, declLoc, name, move(paramsStore), move(body));

            if (isSingletonMethod) {
                ast::cast_tree<ast::MethodDef>(methodExpr)->flags.isSelfMethod = true;
            }

            return methodExpr;
        }
        case PM_DEFINED_NODE: {
            auto definedNode = down_cast<pm_defined_node>(node);

            auto argument = definedNode->value;

            switch (PM_NODE_TYPE(argument)) {
                // Desugar `defined?(@ivar)` to `::Magic.defined_instance_var(:@ivar)`
                case PM_INSTANCE_VARIABLE_READ_NODE: {
                    auto ivarNode = down_cast<pm_instance_variable_read_node>(argument);
                    auto loc = translateLoc(argument->location);
                    auto name = translateConstantName(ivarNode->name);
                    auto sym = MK::Symbol(loc, name);

                    return MK::Send1(loc, MK::Magic(loc), core::Names::definedInstanceVar(),
                                     location.copyWithZeroLength(), move(sym));
                }

                // Desugar `defined?(@@cvar)` to `::Magic.defined_instance_var(:@@cvar)`
                case PM_CLASS_VARIABLE_READ_NODE: {
                    auto cvarNode = down_cast<pm_class_variable_read_node>(argument);
                    auto loc = translateLoc(argument->location);
                    auto name = translateConstantName(cvarNode->name);
                    auto sym = MK::Symbol(loc, name);

                    return MK::Send1(loc, MK::Magic(loc), core::Names::definedClassVar(), location.copyWithZeroLength(),
                                     move(sym));
                }

                // Desugar `defined?(A::B::C)` to `::Magic.defined_p("A", "B", "C")`
                // or `defined?(::A::B::C)` to `::Magic.defined_p()`
                case PM_CONSTANT_READ_NODE:
                case PM_CONSTANT_PATH_NODE: {
                    ast::Send::ARGS_store args;
                    auto current = argument;

                    while (true) {
                        if (PM_NODE_TYPE_P(current, PM_CONSTANT_PATH_NODE)) {
                            auto pathNode = down_cast<pm_constant_path_node>(current);

                            args.emplace_back(MK::String(translateLoc(pathNode->base.location),
                                                         translateConstantName(pathNode->name)));

                            if (pathNode->parent == nullptr) {
                                args.clear();
                                break;
                            }

                            current = pathNode->parent;
                        } else if (PM_NODE_TYPE_P(current, PM_CONSTANT_READ_NODE)) {
                            auto constNode = down_cast<pm_constant_read_node>(current);
                            args.emplace_back(MK::String(translateLoc(constNode->base.location),
                                                         translateConstantName(constNode->name)));
                            break;
                        } else {
                            args.clear();
                            break;
                        }
                    }

                    absl::c_reverse(args);
                    auto argLoc = translateLoc(definedNode->value->location);
                    return MK::Send(argLoc, MK::Magic(argLoc), core::Names::defined_p(), location.copyWithZeroLength(),
                                    args.size(), move(args));
                }
                default: {
                    // All other cases desugar to `::Magic.defined?()` with 0 arguments
                    ast::Send::ARGS_store args;
                    auto argLoc = translateLoc(definedNode->value->location);
                    return MK::Send(argLoc, MK::Magic(argLoc), core::Names::defined_p(), location.copyWithZeroLength(),
                                    args.size(), move(args));
                }
            }
        }
        case PM_ELSE_NODE: { // An `else` clauses, which can pertain to an `if`, `begin`, `case`, etc.
            auto elseNode = down_cast<pm_else_node>(node);
            return desugarStatements(elseNode->statements);
        }
        case PM_EMBEDDED_STATEMENTS_NODE: { // Statements interpolated into a string.
            // e.g. the `#{bar}` in `"foo #{bar} baz"`
            // Can be multiple statements separated by `;`.
            auto embeddedStmtsNode = down_cast<pm_embedded_statements_node>(node);

            auto stmtsNode = embeddedStmtsNode->statements;
            if (stmtsNode == nullptr) {
                return MK::Nil(location);
            }

            auto inlineIfSingle = false;
            return desugarStatements(stmtsNode, inlineIfSingle, location);
        }
        case PM_EMBEDDED_VARIABLE_NODE: {
            auto embeddedVariableNode = down_cast<pm_embedded_variable_node>(node);
            return desugar(embeddedVariableNode->variable);
        }
        case PM_ENSURE_NODE: { // An `ensure` clause, which can pertain to a `begin`
            unreachable("PM_ENSURE_NODE is handled separately as part of PM_BEGIN_NODE, see its docs for details.");
        }
        case PM_FALSE_NODE: { // The `false` keyword
            return MK::False(location);
        }
        case PM_FLOAT_NODE: { // A floating point number literal, e.g. `1.23`
            auto floatNode = down_cast<pm_float_node>(node);
            string valueString(sliceLocation(floatNode->base.location));
            auto withoutUnderscores = absl::StrReplaceAll(valueString, {{"_", ""}});

            double val;
            if (!absl::SimpleAtod(withoutUnderscores, &val)) {
                val = numeric_limits<double>::quiet_NaN();
                if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::FloatOutOfRange)) {
                    e.setHeader("Unsupported float literal: `{}`", valueString);
                    e.addErrorNote("This likely represents a bug in Sorbet. Please report an issue:\n"
                                   "    https://github.com/sorbet/sorbet/issues");
                }
            }

            return MK::Float(location, val);
        }
        case PM_FLIP_FLOP_NODE: { // A flip-flop pattern, like the `flip..flop` in `if flip..flop`
            if (PM_NODE_FLAG_P(node, PM_RANGE_FLAGS_EXCLUDE_END)) { // 3 dots: `flip...flop`
                return make_unsupported_node(location, "EFlipflop");
            } else { // 2 dots: `flip..flop`
                return make_unsupported_node(location, "IFlipflop");
            }
        }
        case PM_FOR_NODE: { // `for x in a; ...; end`
            auto forNode = down_cast<pm_for_node>(node);

            auto *mlhs = PM_NODE_TYPE_P(forNode->index, PM_MULTI_TARGET_NODE)
                             ? down_cast<pm_multi_target_node>(forNode->index)
                             : nullptr;

            auto collection = desugar(forNode->collection);
            auto body = desugarStatements(forNode->statements);

            // Desugar `for x in collection; body; end` into `collection.each { |x| body }`
            bool canProvideNiceDesugar = true;

            // Check if the variable is a simple local variable or a multi-target with only local variables
            if (mlhs) {
                // Multi-target: check if all are local variables (no nested multi-targets or other complex targets)
                auto targets = absl::MakeSpan(mlhs->lefts.nodes, mlhs->lefts.size);
                canProvideNiceDesugar = absl::c_all_of(
                    targets, [](pm_node_t *target) { return PM_NODE_TYPE_P(target, PM_LOCAL_VARIABLE_TARGET_NODE); });
            } else {
                // Single variable: check if it's a local variable
                canProvideNiceDesugar = PM_NODE_TYPE_P(forNode->index, PM_LOCAL_VARIABLE_TARGET_NODE);
            }

            auto locZeroLen = location.copyWithZeroLength();
            ast::MethodDef::PARAMS_store params;

            if (canProvideNiceDesugar) {
                // Simple case: `for x in a; body; end` -> `a.each { |x| body }`
                if (mlhs) {
                    auto targets = absl::MakeSpan(mlhs->lefts.nodes, mlhs->lefts.size);
                    for (auto *target : targets) {
                        params.emplace_back(desugar(target));
                    }
                } else {
                    params.emplace_back(desugar(forNode->index));
                }
            } else {
                // Complex case: `for @x in a; body; end` -> `a.each { || @x = <temp>; body }`
                auto temp = nextUniqueDesugarName(core::Names::forTemp());
                auto tempLocal = MK::Local(location, temp);

                // Desugar the assignment
                ExpressionPtr masgnExpr;
                if (mlhs) {
                    // Multi-target: use desugarMlhs for complex expansion
                    masgnExpr = desugarMlhs(location, mlhs, move(tempLocal));
                } else {
                    // Single variable: simple assignment
                    masgnExpr = MK::Assign(location, desugar(forNode->index), move(tempLocal));
                }

                body = MK::InsSeq1(location, move(masgnExpr), move(body));
            }

            auto block = MK::Block(location, move(body), move(params));
            return MK::Send0Block(location, move(collection), core::Names::each(), locZeroLen, move(block));
        }
        case PM_FORWARDING_ARGUMENTS_NODE: { // The `...` argument in a method call, like `foo(...)`
            unreachable("PM_FORWARDING_ARGUMENTS_NODE is handled separately in `PM_CALL_NODE`.");
        }
        case PM_FORWARDING_PARAMETER_NODE: { // The `...` parameter in a method definition, like `def foo(...)`
            unreachable("PM_FORWARDING_PARAMETER_NODE is handled separately in `desugarParametersNode()`.");
        }
        case PM_FORWARDING_SUPER_NODE: { // A `super` with no explicit arguments
            // It might have a literal block argument, though.

            auto forwardingSuperNode = down_cast<pm_forwarding_super_node>(node);

            // There's no `keyword_loc` field, so we make it ourselves from the start location.
            // constexpr uint32_t length = "super"sv.size();
            // auto keywordLoc = translateLoc(node->location.start, node->location.start + length);

            auto expr = MK::ZSuper(location, maybeTypedSuper());

            auto blockArgumentNode = forwardingSuperNode->block;

            if (blockArgumentNode != nullptr) { // always a PM_BLOCK_NODE
                categoryCounterInc("Prism fallback", "PM_FORWARDING_SUPER_NODE with block");
                throw PrismFallback{}; // TODO: Not implemented yet
            }

            return expr;
        }
        case PM_GLOBAL_VARIABLE_AND_WRITE_NODE: { // And-assignment to a global variable, e.g. `$g &&= false`
            return desugarVariableOpAssign<pm_global_variable_and_write_node, OpAssignKind::And,
                                           ast::UnresolvedIdent::Kind::Global>(node);
        }
        case PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a global variable, e.g. `$g += 1`
            return desugarVariableOpAssign<pm_global_variable_operator_write_node, OpAssignKind::Operator,
                                           ast::UnresolvedIdent::Kind::Global>(node);
        }
        case PM_GLOBAL_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a global variable, e.g. `$g ||= true`
            return desugarVariableOpAssign<pm_global_variable_or_write_node, OpAssignKind::Or,
                                           ast::UnresolvedIdent::Kind::Global>(node);
        }
        case PM_GLOBAL_VARIABLE_READ_NODE: { // A global variable, like `$g`
            auto globalVarReadNode = down_cast<pm_global_variable_read_node>(node);
            auto name = translateConstantName(globalVarReadNode->name);
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Global, name);
        }
        case PM_GLOBAL_VARIABLE_TARGET_NODE: { // Target of an indirect write to a global variable
            // ... like `$target1, $target2 = 1, 2`, `rescue => $target`, etc.
            auto globalVariableTargetNode = down_cast<pm_global_variable_target_node>(node);
            auto name = translateConstantName(globalVariableTargetNode->name);
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Global, name);
        }
        case PM_GLOBAL_VARIABLE_WRITE_NODE: { // Regular assignment to a global variable, e.g. `$g = 1`
            return desugarAssignment<pm_global_variable_write_node, ast::UnresolvedIdent::Kind::Global>(node);
        }
        case PM_HASH_NODE: { // A hash literal, like `{ a: 1, b: 2 }`
            auto hashNode = down_cast<pm_hash_node>(node);

            return desugarKeyValuePairs(location, hashNode->elements);
        }
        case PM_IF_NODE: { // An `if` statement or modifier, like `if cond; ...; end` or `a.b if cond`
            auto ifNode = down_cast<pm_if_node>(node);

            auto predicateExpr = desugar(ifNode->predicate);
            auto thenExpr = desugarStatements(ifNode->statements);
            auto elseExpr = desugarNullable(ifNode->subsequent);

            return MK::If(location, move(predicateExpr), move(thenExpr), move(elseExpr));
        }
        case PM_IMAGINARY_NODE: { // An imaginary number literal, like `1.0i`, `+1.0i`, or `-1.0i`
            auto imaginaryNode = down_cast<pm_imaginary_node>(node);
            // Create a string_view of the value without the trailing 'i'
            auto value = sliceLocation(imaginaryNode->base.location);
            value = value.substr(0, value.size() - 1);

            ENFORCE(!value.empty());

            // Check for optional leading '+' or '-'
            auto sign = value[0];
            bool hasSign = (sign == '+' || sign == '-');

            auto numberLoc = location;
            if (hasSign) {
                value.remove_prefix(1); // Remove the sign

                numberLoc = core::LocOffsets{location.beginPos() + 1, location.endPos()};
            }

            // Create the desugared Complex call: `Kernel.Complex(0, unsigned_value)`
            auto kernel = MK::Constant(numberLoc, core::Symbols::Kernel());
            core::NameRef complexName = core::Names::Constants::Complex().dataCnst(ctx)->original;
            core::NameRef valueName = ctx.state.enterNameUTF8(value);
            auto complexCall = MK::Send2(numberLoc, move(kernel), complexName, numberLoc.copyWithZeroLength(),
                                         MK::Int(numberLoc, 0), MK::String(numberLoc, valueName));

            // If there was a sign, wrap in unary operation
            // E.g. desugar `+42` to `42.+()`
            if (hasSign) {
                core::NameRef unaryOp = (sign == '-') ? core::Names::unaryMinus() : core::Names::unaryPlus();

                return MK::Send0(location, move(complexCall), unaryOp,
                                 core::LocOffsets{location.beginLoc, numberLoc.beginLoc});
            }

            // No leading sign; return the Complex node directly
            return complexCall;
        }
        case PM_IMPLICIT_NODE: { // A hash key without explicit value, like the `k4` in `{ k4: }`
            auto implicitNode = down_cast<pm_implicit_node>(node);
            return desugar(implicitNode->value);
        }
        case PM_IMPLICIT_REST_NODE: { // An implicit splat, like the `,` in `a, = 1, 2, 3`
            auto restLoc = core::LocOffsets{location.beginLoc + 1, location.beginLoc + 1};
            core::NameRef sorbetName = core::Names::restargs();
            return MK::RestParam(restLoc, MK::Local(restLoc, sorbetName));
        }
        case PM_INDEX_AND_WRITE_NODE: { // And-assignment to an index, e.g. `a[i] &&= false`
            return translateIndexAssignment<pm_index_and_write_node, parser::AndAsgn>(node, location);
        }
        case PM_INDEX_OPERATOR_WRITE_NODE: { // Compound assignment to an index, e.g. `a[i] += 1`
            return translateIndexAssignment<pm_index_operator_write_node, parser::OpAsgn>(node, location);
        }
        case PM_INDEX_OR_WRITE_NODE: { // Or-assignment to an index, e.g. `a[i] ||= true`
            return translateIndexAssignment<pm_index_or_write_node, parser::OrAsgn>(node, location);
        }
        case PM_INDEX_TARGET_NODE: { // Target of an indirect write to an indexed expression
            // ... like `target[0], target[1] = 1, 2`, `rescue => target[0]`, etc.
            auto indexedTargetNode = down_cast<pm_index_target_node>(node);

            auto openingLoc = translateLoc(indexedTargetNode->opening_loc);                  // The location of `[]=`
            auto lBracketLoc = core::LocOffsets{openingLoc.beginLoc, openingLoc.endLoc - 1}; // Drop the `=`
            auto receiver = desugar(indexedTargetNode->receiver);

            auto argExprs = desugarArguments<ast::Send::ARGS_store>(indexedTargetNode->arguments);

            return MK::Send(location, move(receiver), core::Names::squareBracketsEq(), lBracketLoc, argExprs.size(),
                            move(argExprs));
        }
        case PM_INSTANCE_VARIABLE_AND_WRITE_NODE: { // And-assignment to an instance variable, e.g. `@iv &&= false`
            return desugarVariableOpAssign<pm_instance_variable_and_write_node, OpAssignKind::And,
                                           ast::UnresolvedIdent::Kind::Instance>(node);
        }
        case PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to an instance variable, e.g. `@iv += 1`
            return desugarVariableOpAssign<pm_instance_variable_operator_write_node, OpAssignKind::Operator,
                                           ast::UnresolvedIdent::Kind::Instance>(node);
        }
        case PM_INSTANCE_VARIABLE_OR_WRITE_NODE: { // Or-assignment to an instance variable, e.g. `@iv ||= true`
            return desugarVariableOpAssign<pm_instance_variable_or_write_node, OpAssignKind::Or,
                                           ast::UnresolvedIdent::Kind::Instance>(node);
        }
        case PM_INSTANCE_VARIABLE_READ_NODE: { // An instance variable, like `@iv`
            auto instanceVarNode = down_cast<pm_instance_variable_read_node>(node);
            auto name = translateConstantName(instanceVarNode->name);
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Instance, name);
        }
        case PM_INSTANCE_VARIABLE_TARGET_NODE: { // Target of an indirect write to an instance variable
            // ... like `@target1, @target2 = 1, 2`, `rescue => @target`, etc.
            auto instanceVariableTargetNode = down_cast<pm_instance_variable_target_node>(node);
            auto name = translateConstantName(instanceVariableTargetNode->name);
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Instance, name);
        }
        case PM_INSTANCE_VARIABLE_WRITE_NODE: { // Regular assignment to an instance variable, e.g. `@iv = 1`
            return desugarAssignment<pm_instance_variable_write_node, ast::UnresolvedIdent::Kind::Instance>(node);
        }
        case PM_INTEGER_NODE: { // An integer literal, e.g., `123`, `0xcafe`, `0b1010`, etc.
            auto intNode = down_cast<pm_integer_node>(node);
            // For normal integers, retain the original valueString including any sign
            string valueString(sliceLocation(intNode->base.location));
            ENFORCE(!valueString.empty());

            // Index to track position in valueString after optional sign
            size_t index = 0;
            auto sign = valueString[0];

            // Check for optional '+' or '-' sign
            if (sign == '+' || sign == '-') {
                index++;
            }

            // Check for prefixed literals starting from the current index
            if (valueString.size() > index + 1 && valueString[index] == '0' &&
                (valueString[index + 1] == '0' || !isdigit(static_cast<unsigned char>(valueString[index + 1])))) {
                // When it comes to prefixed integer literals, Sorbet's parser is inconsistent:
                // If it's a decimal literal, we remove the prefix, but keep the underscore
                // Example: `0d1_23` is 123, needs to be translated to `1_23`
                if (valueString[index + 1] == 'd' || valueString[index + 1] == 'D') {
                    valueString = valueString.substr(index + 2, valueString.size() - index - 2);
                    // But when it's a hexadecimal, octal, or binary literal, Sorbet's parser treats prefixed
                    // literals with underscores as invalid, so we just use `0`
                    // Example: `0xca_fe` is invalid, should be translated to `0`
                    // Note: Prism actually parses these literals correctly
                } else if (valueString.find('_') != string::npos) {
                    valueString = "0";
                } else {
                    // Handle prefixed integer literals (e.g., 0x, 0b, 0o, 0d) without underscores
                    // Prism has already parsed their values so we use the precomputed value directly
                    valueString = to_string(intNode->value.value);
                }

                // Add the optional sign back if it was present
                if (index != 0) {
                    valueString = sign + valueString;
                }
            }

            auto underscorePos = valueString.find("_");
            const string &withoutUnderscores =
                (underscorePos == string::npos) ? valueString : absl::StrReplaceAll(valueString, {{"_", ""}});

            int64_t val;
            if (!absl::SimpleAtoi(withoutUnderscores, &val)) {
                val = 0;
                if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::IntegerOutOfRange)) {
                    e.setHeader("Unsupported integer literal: `{}`", valueString);
                }
            }

            return MK::Int(location, val);
        }
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE: { // An interpolated regex literal in a conditional...
            // ...that implicitly checks against the last read line by an IO object, e.g. `if /wat #{123}/`
            return make_unsupported_node(location, "MatchCurLine");
        }
        case PM_INTERPOLATED_REGULAR_EXPRESSION_NODE: { // A regular expression with interpolation, like `/a #{b} c/`
            auto interpolatedRegexNode = down_cast<pm_interpolated_regular_expression_node>(node);

            auto options = desugarRegexpOptions(interpolatedRegexNode->closing_loc);

            // Desugar interpolated regexp to Regexp.new(pattern, options)
            auto pattern = desugarDString(location, interpolatedRegexNode->parts);

            auto cnst = MK::Constant(location, core::Symbols::Regexp());
            return MK::Send2(location, move(cnst), core::Names::new_(), location.copyWithZeroLength(), move(pattern),
                             move(options));
        }
        case PM_INTERPOLATED_STRING_NODE: { // An interpolated string like `"foo #{bar} baz"`
            auto interpolatedStringNode = down_cast<pm_interpolated_string_node>(node);

            // Desugar `"a #{b} c"` to `::Magic.<string-interpolate>("a ", b, " c")`
            return desugarDString(location, interpolatedStringNode->parts);
        }
        case PM_INTERPOLATED_SYMBOL_NODE: { // A symbol like `:"a #{b} c"`
            auto interpolatedSymbolNode = down_cast<pm_interpolated_symbol_node>(node);

            // Desugar `:"a #{b} c"` to `::Magic.<string-interpolate>("a ", b, " c").intern()`
            auto desugared = desugarDString(location, interpolatedSymbolNode->parts);
            return MK::Send0(location, move(desugared), core::Names::intern(), location.copyWithZeroLength());
        }
        case PM_INTERPOLATED_X_STRING_NODE: { // An executable string with backticks, like `echo "Hello, world!"`
            auto interpolatedXStringNode = down_cast<pm_interpolated_x_string_node>(node);
            auto desugared = desugarDString(location, interpolatedXStringNode->parts);
            return MK::Send1(location, MK::Self(location), core::Names::backtick(), location.copyWithZeroLength(),
                             move(desugared));
        }
        case PM_IT_LOCAL_VARIABLE_READ_NODE: { // Reading the `it` default param in a block, e.g. `a.map { it + 1 }`
            return MK::Local(location, core::Names::it());
        }
        case PM_IT_PARAMETERS_NODE: { // An invisible node that models the 'it' parameter in a block/lambda
            // ... for a block like `proc { it + 1 }` or lambda like `-> { it + 1 }`, which has no explicitly declared
            // parameters. When translating this node directly (not through the translateCallWithBlock handler), we
            // don't have access to the block body to find actual usage locations via AST walking.
            auto itParamLoc = location.copyWithZeroLength();

            // Single 'it' parameter - use the original name (not a unique one)
            // Unlike numbered parameters, 'it' uses the actual name "it" so that
            // local variables named 'it' in the same scope can shadow it
            return MK::Local(itParamLoc, core::Names::it());
        }
        case PM_KEYWORD_HASH_NODE: { // A hash of keyword arguments, like `foo(a: 1, b: 2)`
            auto keywordHashNode = down_cast<pm_keyword_hash_node>(node);
            return desugarKeyValuePairs(location, keywordHashNode->elements);
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: { // A keyword rest parameter, like `def foo(**kwargs)`
            // This doesn't include `**nil`, which is a `PM_NO_KEYWORDS_PARAMETER_NODE`.
            auto keywordRestParamNode = down_cast<pm_keyword_rest_parameter_node>(node);

            core::NameRef sorbetName;
            core::LocOffsets kwrestLoc;
            if (auto prismName = keywordRestParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named keyword rest parameter, like `def foo(**kwargs)`
                sorbetName = translateConstantName(prismName);

                // The location doesn't include the `**`, only the splatted expression like `kwargs` in `**kwargs`
                constexpr uint32_t length = "**"sv.size();
                kwrestLoc = core::LocOffsets{location.beginPos() + length, location.endPos()};
            } else { // An anonymous keyword rest parameter, like `def foo(**)`
                sorbetName = nextUniqueParserName(core::Names::starStar());

                // This location *does* include the whole `**`.
                kwrestLoc = location;
            }

            return MK::RestParam(kwrestLoc, MK::KeywordArg(kwrestLoc, sorbetName));
        }
        case PM_LAMBDA_NODE: { // lambda literals, like `-> { 123 }`
            auto lambdaNode = down_cast<pm_lambda_node>(node);

            auto operatorLoc = translateLoc(lambdaNode->operator_loc); // the `->` arrow

            // `-> { "block" }` or `-> do "block" end`
            //     ^^^^^^^^^^^         ^^^^^^^^^^^^^^
            auto blockLoc = pm_location_t{.start = lambdaNode->opening_loc.start, .end = lambdaNode->closing_loc.end};

            auto receiver = MK::Constant(operatorLoc, core::Symbols::Kernel());
            pm_arguments_node *args = nullptr;
            auto block = DesugaredBlockArgument::literalBlock(
                desugarLiteralBlock(lambdaNode->body, lambdaNode->parameters, blockLoc, lambdaNode->operator_loc));
            auto isPrivateOk = false; // `Kernel.lambda` is not a private call
            return desugarMethodCall(move(receiver), core::Names::lambda(), operatorLoc, args, lambdaNode->closing_loc,
                                     move(block), location, isPrivateOk);
        }
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE: { // And-assignment to a local variable, e.g. `local &&= false`
            return desugarVariableOpAssign<pm_local_variable_and_write_node, OpAssignKind::And,
                                           ast::UnresolvedIdent::Kind::Local>(node);
        }
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a local variable, e.g. `local += 1`
            return desugarVariableOpAssign<pm_local_variable_operator_write_node, OpAssignKind::Operator,
                                           ast::UnresolvedIdent::Kind::Local>(node);
        }
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a local variable, e.g. `local ||= true`
            return desugarVariableOpAssign<pm_local_variable_or_write_node, OpAssignKind::Or,
                                           ast::UnresolvedIdent::Kind::Local>(node);
        }
        case PM_LOCAL_VARIABLE_READ_NODE: { // A local variable, like `lv`
            auto localVarReadNode = down_cast<pm_local_variable_read_node>(node);
            auto name = translateConstantName(localVarReadNode->name);
            return MK::Local(location, name);
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // Target of an indirect write to a local variable
            // ... like `target1, target2 = 1, 2`, `rescue => target`, etc.
            auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
            auto name = translateConstantName(localVarTargetNode->name);
            return MK::Local(location, name);
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: { // Regular assignment to a local variable, e.g. `local = 1`
            return desugarAssignment<pm_local_variable_write_node, ast::UnresolvedIdent::Kind::Local>(node);
        }
        case PM_MATCH_LAST_LINE_NODE: { // A regex literal in a conditional...
            // ...that implicitly checks against the last read line by an IO object, e.g. `if /wat/`
            return make_unsupported_node(location, "MatchCurLine");
        }
        case PM_MATCH_REQUIRED_NODE: {
            auto matchRequiredNode = down_cast<pm_match_required_node>(node);

            auto value = desugarPattern(matchRequiredNode->value);
            auto pattern = desugarPattern(matchRequiredNode->pattern);

            return desugarOnelinePattern(location, matchRequiredNode->pattern);
        }
        case PM_MATCH_PREDICATE_NODE: {
            auto matchPredicateNode = down_cast<pm_match_predicate_node>(node);

            auto value = desugarPattern(matchPredicateNode->value);
            auto pattern = desugarPattern(matchPredicateNode->pattern);

            return desugarOnelinePattern(location, matchPredicateNode->pattern);
        }
        case PM_MATCH_WRITE_NODE: { // A regex match that assigns to a local variable, like `a =~ /wat/`
            auto matchWriteNode = down_cast<pm_match_write_node>(node);

            // "Match writes" let you bind regex capture groups directly into new variables.
            // Sorbet doesn't treat this syntax in a special way, so it doesn't know that it introduces new local var.
            // It's treated as a normal call to `=~` with a Regexp receiver and the rhs as an argument.
            //
            // This is why we just translate the `call` and completely ignore `matchWriteNode->targets`.

            return desugar(up_cast(matchWriteNode->call));
        }
        case PM_MODULE_NODE: { // Modules declarations, like `module A::B::C; ...; end`
            auto moduleNode = down_cast<pm_module_node>(node);

            auto name = desugar(moduleNode->constant_path);
            auto declLoc = translateLoc(moduleNode->module_keyword_loc).join(name.loc());

            auto body = this->enterModuleContext(enclosingBlockParamLoc, enclosingBlockParamName)
                            .desugarClassOrModule(moduleNode->body);

            return MK::Module(location, declLoc, move(name), move(body));
        }
        case PM_MULTI_TARGET_NODE: { // A multi-target like the `(x2, y2)` in `p1, (x2, y2) = a`
            auto multiTargetNode = down_cast<pm_multi_target_node>(node);

            return desugarMlhs(location, multiTargetNode, MK::EmptyTree());
        }
        case PM_MULTI_WRITE_NODE: { // Multi-assignment, like `a, b = 1, 2`
            auto multiWriteNode = down_cast<pm_multi_write_node>(node);

            auto rhsExpr = desugar(multiWriteNode->value);

            return desugarMlhs(location, multiWriteNode, move(rhsExpr));
        }
        case PM_NEXT_NODE: { // A `next` statement, e.g. `next`, `next 1, 2, 3`
            auto nextNode = down_cast<pm_next_node>(node);
            auto arguments = desugarBreakNextReturn(nextNode->arguments);
            return MK::Next(location, move(arguments));
        }
        case PM_NIL_NODE: { // The `nil` keyword
            return MK::Nil(location);
        }
        case PM_NO_KEYWORDS_PARAMETER_NODE: { // `**nil`, such as in `def foo(**nil)` or `h in { k: v, **nil}`
            unreachable("PM_NO_KEYWORDS_PARAMETER_NODE is handled separately in `PM_HASH_PATTERN_NODE` and "
                        "`PM_PARAMETERS_NODE`.");
        }
        case PM_NUMBERED_PARAMETERS_NODE: { // An invisible node that models the numbered parameters in a block
            // ... for a block like `proc { _1 + _2 }`, which has no explicitly declared parameters.
            unreachable("PM_NUMBERED_PARAMETERS_NODE is handled separately in `translateNumberedParametersNode()`.");
        }
        case PM_NUMBERED_REFERENCE_READ_NODE: {
            auto numberedReferenceReadNode = down_cast<pm_numbered_reference_read_node>(node);
            auto number = numberedReferenceReadNode->number;

            auto name = ctx.state.enterNameUTF8(to_string(number));
            return ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Global, name);
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: { // An optional keyword parameter, like `def foo(a: 1)`
            auto optionalKeywordParamNode = down_cast<pm_optional_keyword_parameter_node>(node);
            auto nameLoc = translateLoc(optionalKeywordParamNode->name_loc);

            auto name = translateConstantName(optionalKeywordParamNode->name);
            auto value = desugar(optionalKeywordParamNode->value);

            return MK::OptionalParam(location, MK::KeywordArg(nameLoc, name), move(value));
        }
        case PM_OPTIONAL_PARAMETER_NODE: { // An optional positional parameter, like `def foo(a = 1)`
            auto optionalParamNode = down_cast<pm_optional_parameter_node>(node);
            auto nameLoc = translateLoc(optionalParamNode->name_loc);

            auto name = translateConstantName(optionalParamNode->name);
            auto value = desugar(optionalParamNode->value);

            return MK::OptionalParam(location, MK::Local(nameLoc, name), move(value));
        }
        case PM_OR_NODE: { // operator `||` and `or`
            auto orNode = down_cast<pm_or_node>(node);

            auto lhs = desugarNullable(orNode->left);
            auto rhs = desugarNullable(orNode->right);

            if (preserveConcreteSyntax) {
                auto orOrLoc = core::LocOffsets{lhs.loc().endPos(), rhs.loc().beginPos()};
                return MK::Send2(location, MK::Magic(location.copyWithZeroLength()), core::Names::orOr(), orOrLoc,
                                 move(lhs), move(rhs));
            }

            if (isa_reference(lhs)) {
                auto cond = MK::cpRef(lhs);
                return MK::If(location, move(cond), move(lhs), move(rhs));
            }

            // For non-reference expressions, create a temporary variable so we don't evaluate the LHS twice.
            // E.g. `x = 1 || 2` becomes `x = (temp = 1; temp ? temp : 2)`
            core::NameRef tempLocalName = nextUniqueDesugarName(core::Names::orOr());
            auto lhsLoc = lhs.loc();
            auto rhsLoc = rhs.loc();
            auto condLoc =
                lhsLoc.exists() && rhsLoc.exists() ? core::LocOffsets{lhsLoc.endPos(), rhsLoc.beginPos()} : lhsLoc;
            auto tempAssign = MK::Assign(location, tempLocalName, move(lhs));
            auto cond = MK::Local(condLoc, tempLocalName);
            auto thenp = MK::Local(lhsLoc, tempLocalName);
            auto if_ = MK::If(location, move(cond), move(thenp), move(rhs));
            return MK::InsSeq1(location, move(tempAssign), move(if_));
        }
        case PM_PARAMETERS_NODE: { // The parameters declared at the top of a PM_DEF_NODE
            unreachable("PM_PARAMETERS_NODE is handled separately in desugarParametersNode.");
        }
        case PM_PARENTHESES_NODE: { // A parethesized expression, e.g. `(a)`
            auto parensNode = down_cast<pm_parentheses_node>(node);

            auto stmtsNode = parensNode->body;

            if (stmtsNode == nullptr) {
                return MK::Nil(location);
            }

            if (PM_NODE_TYPE_P(stmtsNode, PM_STATEMENTS_NODE)) {
                auto inlineIfSingle = false;
                return desugarStatements(down_cast<pm_statements_node>(stmtsNode), inlineIfSingle);
            } else {
                return desugar(stmtsNode);
            }
        }
        case PM_PRE_EXECUTION_NODE: { // The BEGIN keyword and body, like `BEGIN { ... }`
            return make_unsupported_node(location, "Preexe");
        }
        case PM_PROGRAM_NODE: { // The root node of the parse tree, representing the entire program
            pm_program_node *programNode = down_cast<pm_program_node>(node);

            // Report parse errors once at the root level
            for (auto &error : parseErrors) {
                // EOF error is always pointed to the very last line of the file, which can't be expressed in Sorbet's
                // error comments
                if (error.id != PM_ERR_UNEXPECTED_TOKEN_CLOSE_CONTEXT) {
                    reportError(translateLoc(error.location), error.message);
                }
            }

            return desugarStatements(programNode->statements);
        }
        case PM_POST_EXECUTION_NODE: { // The END keyword and body, like `END { ... }`
            return make_unsupported_node(location, "Postexe");
        }
        case PM_RANGE_NODE: { // A Range literal, e.g. `a..b`, `a..`, `..b`, `a...b`, `a...`, `...b`
            auto rangeNode = down_cast<pm_range_node>(node);

            bool isExclusive = PM_NODE_FLAG_P(rangeNode, PM_RANGE_FLAGS_EXCLUDE_END);

            auto recv = MK::Magic(location);
            auto locZeroLen = core::LocOffsets{location.beginPos(), location.beginPos()};

            auto fromExpr = desugarNullable(rangeNode->left);
            auto toExpr = desugarNullable(rangeNode->right);

            auto excludeEndExpr = isExclusive ? MK::True(location) : MK::False(location);

            // Desugar to `::Kernel.<buildRange>(from, to, excludeEnd)`
            return MK::Send3(location, move(recv), core::Names::buildRange(), locZeroLen, move(fromExpr), move(toExpr),
                             move(excludeEndExpr));
        }
        case PM_RATIONAL_NODE: { // A rational number literal, e.g. `1r`
            // Note: in `1/2r`, only the `2r` is part of the `PM_RATIONAL_NODE`.
            // The `1/` is just divison of an integer.
            auto *rationalNode = down_cast<pm_rational_node>(node);

            // `-1` drops the trailing `r` end of the value
            auto value = sliceLocation(rationalNode->base.location);
            value = value.substr(0, value.size() - 1);

            auto kernel = MK::Constant(location, core::Symbols::Kernel());
            core::NameRef rationalName = core::Names::Constants::Rational().dataCnst(ctx)->original;
            core::NameRef valueName = ctx.state.enterNameUTF8(value);

            // Desugar to `123r` to `::Kernel.Rational("123")`
            return MK::Send1(location, move(kernel), rationalName, location.copyWithZeroLength(),
                             MK::String(location, valueName));
        }
        case PM_REDO_NODE: { // The `redo` keyword
            return make_unsupported_node(location, "Redo");
        }
        case PM_REGULAR_EXPRESSION_NODE: { // A regular expression literal, e.g. `/foo/`
            auto regexNode = down_cast<pm_regular_expression_node>(node);

            auto contentLoc = translateLoc(regexNode->content_loc);

            return desugarRegexp(location, contentLoc, regexNode->unescaped, regexNode->closing_loc);
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: { // A required keyword parameter, like `def foo(a:)`
            auto requiredKeywordParamNode = down_cast<pm_required_keyword_parameter_node>(node);
            auto name = translateConstantName(requiredKeywordParamNode->name);
            return MK::KeywordArg(location, name);
        }
        case PM_REQUIRED_PARAMETER_NODE: { // A required positional parameter, like `def foo(a)`
            auto requiredParamNode = down_cast<pm_required_parameter_node>(node);
            auto name = translateConstantName(requiredParamNode->name);
            return MK::Local(location, name);
        }
        case PM_RESCUE_MODIFIER_NODE: {
            auto rescueModifierNode = down_cast<pm_rescue_modifier_node>(node);
            auto keywordLoc = translateLoc(rescueModifierNode->keyword_loc);

            // Create a RescueCase with empty exceptions and a <rescueTemp> variable
            ast::RescueCase::EXCEPTION_store exceptions;
            auto rescueTemp = nextUniqueDesugarName(core::Names::rescueTemp());

            auto resbodyLoc = core::LocOffsets{keywordLoc.beginPos(), location.endPos()};

            auto bodyExpr = desugar(rescueModifierNode->expression);
            auto rescueExpr = desugar(rescueModifierNode->rescue_expression);

            auto rescueCaseLoc =
                translateLoc(rescueModifierNode->keyword_loc.start, rescueModifierNode->base.location.end);
            auto rescueCase = ast::make_expression<ast::RescueCase>(
                rescueCaseLoc, move(exceptions), ast::MK::Local(keywordLoc, rescueTemp), move(rescueExpr));

            ast::Rescue::RESCUE_CASE_store rescueCases;
            rescueCases.emplace_back(move(rescueCase));
            return ast::make_expression<ast::Rescue>(location, move(bodyExpr), move(rescueCases), ast::MK::EmptyTree(),
                                                     ast::MK::EmptyTree());
        }
        case PM_RESCUE_NODE: {
            unreachable("PM_RESCUE_NODE is handled separately in PM_BEGIN_NODE, see its docs for details.");
        }
        case PM_REST_PARAMETER_NODE: { // A rest parameter, like `def foo(*rest)`
            auto restParamNode = down_cast<pm_rest_parameter_node>(node);
            core::LocOffsets nameLoc;

            core::NameRef sorbetName;
            if (auto prismName = restParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named rest parameter, like `def foo(*rest)`
                sorbetName = translateConstantName(prismName);
                nameLoc = translateLoc(restParamNode->name_loc);
            } else { // An anonymous rest parameter, like `def foo(*)`
                sorbetName = core::Names::star();
                nameLoc = location;
            }

            return MK::RestParam(location, MK::Local(nameLoc, sorbetName));
        }
        case PM_RETURN_NODE: { // A `return` statement, like `return 1, 2, 3`
            auto returnNode = down_cast<pm_return_node>(node);
            auto arguments = desugarBreakNextReturn(returnNode->arguments);
            return MK::Return(location, move(arguments));
        }
        case PM_RETRY_NODE: { // The `retry` keyword
            return ast::make_expression<ast::Retry>(location);
        }
        case PM_SELF_NODE: { // The `self` keyword
            return MK::Self(location);
        }
        case PM_SHAREABLE_CONSTANT_NODE: {
            // Sorbet doesn't handle `shareable_constant_value` yet (https://bugs.ruby-lang.org/issues/17273).
            // We'll just handle the inner constant assignment as normal.
            auto shareableConstantNode = down_cast<pm_shareable_constant_node>(node);
            return desugar(shareableConstantNode->write);
        }
        case PM_SINGLETON_CLASS_NODE: { // A singleton class, like `class << self ... end`
            auto classNode = down_cast<pm_singleton_class_node>(node);

            auto declLoc = translateLoc(classNode->class_keyword_loc);
            auto receiverLoc = translateLoc(classNode->expression->location);

            if (!PM_NODE_TYPE_P(classNode->expression, PM_SELF_NODE)) { // Only `class << self` is supported
                if (auto e = ctx.beginIndexerError(receiverLoc, core::errors::Desugar::InvalidSingletonDef)) {
                    e.setHeader("`{}` is only supported for `{}`", "class << EXPRESSION", "class << self");
                }
                return MK::EmptyTree();
            }

            auto body = this->enterClassContext(enclosingBlockParamLoc, enclosingBlockParamName)
                            .desugarClassOrModule(classNode->body);

            // Singleton classes are modelled as a class with a special name `<singleton>`
            auto singletonClassName = ast::make_expression<ast::UnresolvedIdent>(
                receiverLoc, ast::UnresolvedIdent::Kind::Class, core::Names::singleton());

            return MK::Class(location, declLoc, move(singletonClassName), ast::ClassDef::ANCESTORS_store{}, move(body));
        }
        case PM_SOURCE_ENCODING_NODE: { // The `__ENCODING__` keyword
            return MK::Send0(location, MK::Magic(location), core::Names::getEncoding(), location.copyWithZeroLength());
        }
        case PM_SOURCE_FILE_NODE: { // The `__FILE__` keyword
            return MK::String(location, core::Names::currentFile());
        }
        case PM_SOURCE_LINE_NODE: { // The `__LINE__` keyword
            auto details = ctx.locAt(location).toDetails(ctx);
            ENFORCE(details.first.line == details.second.line, "position corrupted");

            return MK::Int(location, details.first.line);
        }
        case PM_SPLAT_NODE: { // A splat, like `*a` in an array literal or method call
            auto splatNode = down_cast<pm_splat_node>(node);

            auto expr = desugarNullable(splatNode->expression);

            if (ast::isa_tree<ast::EmptyTree>(expr)) { // An anonymous splat like `f(*)`
                auto var = MK::Local(location, core::Names::star());
                return MK::Splat(location, move(var));
            } else { // Splatting an expression like `f(*a)`
                return MK::Splat(location, move(expr));
            }
        }
        case PM_STATEMENTS_NODE: { // A sequence of statements, such a in a `begin` block, `()`, etc.
            auto statementsNode = down_cast<pm_statements_node>(node);
            return desugarStatements(statementsNode);
        }
        case PM_STRING_NODE: { // A string literal, e.g. `"foo"`
            auto strNode = down_cast<pm_string_node>(node);

            auto unescaped = &strNode->unescaped;
            auto content = ctx.state.enterNameUTF8(parser.extractString(unescaped));

            return MK::String(location, content);
        }
        case PM_SUPER_NODE: { // A `super` call with explicit args, like `super()`, `super(a, b)`
            // If there's no arguments (except a literal block argument), then it's a `PM_FORWARDING_SUPER_NODE`.
            categoryCounterInc("Prism fallback", "PM_SUPER_NODE");
            throw PrismFallback{};
        }
        case PM_SYMBOL_NODE: { // A symbol literal, e.g. `:foo`, or `a:` in `{a: 1}`
            auto symNode = down_cast<pm_symbol_node>(node);

            auto [content, location] = translateSymbol(symNode);

            return MK::Symbol(location, content);
        }
        case PM_TRUE_NODE: { // The `true` keyword
            return MK::True(location);
        }
        case PM_UNDEF_NODE: { // The `undef` keyword, like `undef :method_to_undef
            auto undefNode = down_cast<pm_undef_node>(node);

            ENFORCE(undefNode->names.size > 0, "PM_UNDEF_NODE without names is expected to be a parse error");

            auto args = nodeListToStore<ast::Send::ARGS_store>(undefNode->names);
            auto expr = MK::Send(location, MK::Constant(location, core::Symbols::Kernel()), core::Names::undef(),
                                 location.copyWithZeroLength(), args.size(), std::move(args));
            // It wasn't a Send to begin with--there's no way this could result in a private
            // method call error.
            ast::cast_tree_nonnull<ast::Send>(expr).flags.isPrivateOk = true;
            return expr;
        }
        case PM_UNLESS_NODE: { // An `unless` branch, either in a statement or modifier form.
            auto unlessNode = down_cast<pm_unless_node>(node);

            auto predicateExpr = desugar(unlessNode->predicate);
            // For `unless`, then/else are swapped: `statements` is the else branch, `else_clause` is the then branch
            auto elseExpr = desugarStatements(unlessNode->statements);
            ExpressionPtr thenExpr = desugarNullable(up_cast(unlessNode->else_clause));

            return MK::If(location, move(predicateExpr), move(thenExpr), move(elseExpr));
        }
        case PM_UNTIL_NODE: { // A `until` loop, like `until stop_condition; ...; end`
            auto untilNode = down_cast<pm_until_node>(node);

            // When the until loop is placed after a `begin` block, like `begin; end until false`,
            bool beginModifier = PM_NODE_FLAG_P(untilNode, PM_LOOP_FLAGS_BEGIN_MODIFIER);

            auto cond = desugar(untilNode->predicate);
            auto body = desugarStatements(untilNode->statements);

            if (beginModifier) {
                auto breaker = MK::If(location, std::move(cond), MK::Break(location, MK::EmptyTree()), MK::EmptyTree());
                auto breakWithBody = MK::InsSeq1(location, std::move(body), std::move(breaker));
                return MK::While(location, MK::True(location), std::move(breakWithBody));
            } else {
                // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                auto negatedCond =
                    MK::Send0(location, std::move(cond), core::Names::bang(), location.copyWithZeroLength());
                return MK::While(location, std::move(negatedCond), std::move(body));
            }
        }
        case PM_WHEN_NODE: { // A `when` clause, as part of a `case` statement
            unreachable("`PM_WHEN_NODE` is handled separately in `PM_CASE_NODE`.");
        }
        case PM_WHILE_NODE: { // A `while` loop, like `while condition; ...; end`
            auto whileNode = down_cast<pm_while_node>(node);

            // When the while loop is placed after a `begin` block, like `begin; end while false`,
            bool beginModifier = PM_NODE_FLAG_P(whileNode, PM_LOOP_FLAGS_BEGIN_MODIFIER);

            auto cond = desugar(whileNode->predicate);
            auto body = desugarStatements(whileNode->statements);

            if (beginModifier) {
                // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                auto negatedCond =
                    MK::Send0(location, std::move(cond), core::Names::bang(), location.copyWithZeroLength());
                auto breaker =
                    MK::If(location, std::move(negatedCond), MK::Break(location, MK::EmptyTree()), MK::EmptyTree());
                auto breakWithBody = MK::InsSeq1(location, std::move(body), std::move(breaker));
                return MK::While(location, MK::True(location), std::move(breakWithBody));
            } else {
                return MK::While(location, std::move(cond), std::move(body));
            }
        }
        case PM_X_STRING_NODE: { // A non-interpolated x-string, like `/usr/bin/env ls`
            auto strNode = down_cast<pm_x_string_node>(node);

            auto unescaped = &strNode->unescaped;
            auto source = parser.extractString(unescaped);
            auto content = ctx.state.enterNameUTF8(source);
            auto contentLoc = translateLoc(strNode->content_loc);

            // Create the backtick send call for the desugared expression
            return MK::Send1(location, MK::Self(location), core::Names::backtick(), location.copyWithZeroLength(),
                             MK::String(contentLoc, content));
        }
        case PM_YIELD_NODE: { // The `yield` keyword, like `yield`, `yield 1, 2, 3`
            auto yieldNode = down_cast<pm_yield_node>(node);

            auto yieldArgs = desugarArguments<ast::Send::ARGS_store>(yieldNode->arguments);

            ExpressionPtr recv;
            if (this->enclosingBlockParamName.exists()) {
                if (this->enclosingBlockParamName == core::Names::blkArg()) {
                    this->enclosingBlockParamLoc = location;
                    this->enclosingBlockParamName = core::Names::implicitYield();
                }
                recv = MK::Local(location, this->enclosingBlockParamName);
            } else {
                recv = MK::RaiseUnimplemented(location);
            }

            return MK::Send(location, std::move(recv), core::Names::call(), location.copyWithZeroLength(),
                            yieldArgs.size(), std::move(yieldArgs));
        }

        case PM_ALTERNATION_PATTERN_NODE: // A pattern like `1 | 2`
        case PM_ARRAY_PATTERN_NODE:       // An array pattern such as the `[head, *tail]` in the `a in [head, *tail]`
        case PM_CAPTURE_PATTERN_NODE:     // A variable capture such as the `=> i` in `in Integer => i`
        case PM_FIND_PATTERN_NODE:        // A find pattern such as the `[*, middle, *]` in the `a in [*, middle, *]`
        case PM_HASH_PATTERN_NODE:        // An hash pattern such as the `{ k: Integer }` in the `h in { k: Integer }`
        case PM_IN_NODE:                // An `in` pattern such as in a `case` statement, or as a standalone expression.
        case PM_PINNED_EXPRESSION_NODE: // A "pinned" expression, like `^(1 + 2)` in `in ^(1 + 2)`
        case PM_PINNED_VARIABLE_NODE:   // A "pinned" variable, like `^x` in `in ^x`
            unreachable("These pattern-match related nodes are handled separately in `Translator::desugarPattern()`.");

        case PM_SCOPE_NODE: // An internal node type only created by the MRI's Ruby compiler, and not Prism itself.
            unreachable("Prism's parser never produces `PM_SCOPE_NODE` nodes.");

        case PM_MISSING_NODE: {
            return MK::UnresolvedConstant(location, MK::EmptyTree(), core::Names::Constants::ErrorNode());
        }
    }
}

core::LocOffsets Translator::translateLoc(const uint8_t *start, const uint8_t *end) const {
    return parser.translateLocation(start, end);
}

core::LocOffsets Translator::translateLoc(pm_location_t loc) const {
    return parser.translateLocation(loc);
}

// Similar to `desugar()`, but it's used for pattern-matching nodes.
//
// This is necessary because some Prism nodes get translated differently depending on whether they're part of "regular"
// syntax, or pattern-matching syntax.
//
// E.g. `PM_LOCAL_VARIABLE_TARGET_NODE` normally translates to `parser::LVarLhs`, but `parser::MatchVar` in the context
// of a pattern.
ast::ExpressionPtr Translator::desugarPattern(pm_node_t *node) {
    if (node == nullptr)
        return nullptr;

    auto location = translateLoc(node->location);

    switch (PM_NODE_TYPE(node)) {
        case PM_ALTERNATION_PATTERN_NODE: { // A pattern like `1 | 2`
            auto alternationPatternNode = down_cast<pm_alternation_pattern_node>(node);

            auto left = desugarPattern(alternationPatternNode->left);
            auto right = desugarPattern(alternationPatternNode->right);

            // Like array/hash patterns, MatchAlt is a structural pattern that doesn't have
            // a simple desugared expression - it's handled specially during pattern matching desugaring
            return MK::Nil(location);
        }
        case PM_ASSOC_NODE: { // A key-value pair in a Hash pattern, e.g. the `k: v` in `h in { k: v }
            auto assocNode = down_cast<pm_assoc_node>(node);

            // If the value is an implicit node, skip creating the pair, and return that value directly.
            if (PM_NODE_TYPE_P(assocNode->value, PM_IMPLICIT_NODE)) {
                return desugarPattern(assocNode->value);
            }

            auto key = desugar(assocNode->key);
            auto value = desugarPattern(assocNode->value);

            // Pair is a structural component of hash patterns with no simple desugared expression
            return MK::Nil(location);
        }
        case PM_ARRAY_PATTERN_NODE: { // An array pattern such as the `[head, *tail]` in the `a in [head, *tail]`
            auto arrayPatternNode = down_cast<pm_array_pattern_node>(node);

            auto prismPrefixNodes = absl::MakeSpan(arrayPatternNode->requireds.nodes, arrayPatternNode->requireds.size);
            auto prismRestNode = arrayPatternNode->rest;
            auto prismSuffixNodes = absl::MakeSpan(arrayPatternNode->posts.nodes, arrayPatternNode->posts.size);

            for (auto *prismNode : prismPrefixNodes) {
                desugarPattern(prismNode);
            }

            // Implicit rest nodes in array patterns don't need to be translated
            if (prismRestNode != nullptr && !PM_NODE_TYPE_P(prismRestNode, PM_IMPLICIT_REST_NODE)) {
                desugarPattern(prismRestNode);
            }

            for (auto *prismNode : prismSuffixNodes) {
                desugarPattern(prismNode);
            }

            // Determine the correct location for the pattern
            // If there's a constant, the pattern location excludes it
            core::LocOffsets patternLoc = location;
            if (arrayPatternNode->constant) {
                ENFORCE(arrayPatternNode->opening_loc.start && arrayPatternNode->closing_loc.end,
                        "Array pattern without parentheses or square brackets?");

                // The `ArrayPattern` loc doesn't include the constant, if there is one.
                //     `Point[x: Integer => 1, y: Integer => 2]`
                //           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ArrayPattern loc
                //      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ConstPattern loc
                patternLoc = translateLoc(arrayPatternNode->opening_loc.start, arrayPatternNode->closing_loc.end);
            }

            if (auto *prismConstant = arrayPatternNode->constant) {
                // An array pattern can start with a constant that matches against a specific type,
                // (rather than any value whose `#deconstruct` results are matched by the pattern).
                // E.g. the `Point` in `in Point[1, 2]`
                auto sorbetConstant = desugar(prismConstant);

                // ConstPattern wrapping the array pattern - the desugared expression is Nil as it's structural
                return MK::Nil(location);
            }

            return MK::Nil(patternLoc);
        }
        case PM_CAPTURE_PATTERN_NODE: { // A variable capture such as the `Integer => i` in `in Integer => i`
            auto capturePatternNode = down_cast<pm_capture_pattern_node>(node);

            auto pattern = desugarPattern(capturePatternNode->value);
            auto target = desugarPattern(up_cast(capturePatternNode->target));

            return MK::Nil(location);
        }
        case PM_FIND_PATTERN_NODE: { // A find pattern such as the `[*, middle, *]` in the `a in [*, middle, *]`
            auto findPatternNode = down_cast<pm_find_pattern_node>(node);

            auto prismLeadingSplat = findPatternNode->left;
            auto prismMiddleNodes = absl::MakeSpan(findPatternNode->requireds.nodes, findPatternNode->requireds.size);
            auto prismTrailingSplat = findPatternNode->right;

            if (prismLeadingSplat != nullptr) {
                desugarPattern(up_cast(prismLeadingSplat));
            }

            for (auto *prismNode : prismMiddleNodes) {
                desugarPattern(prismNode);
            }

            if (prismTrailingSplat != nullptr && PM_NODE_TYPE_P(prismTrailingSplat, PM_SPLAT_NODE)) {
                // TODO: handle PM_NODE_TYPE_P(prismTrailingSplat, PM_MISSING_NODE)
                desugarPattern(prismTrailingSplat);
            }

            // FindPattern is a structural pattern with no simple desugared expression
            return MK::Nil(location);
        }
        case PM_HASH_PATTERN_NODE: { // An hash pattern such as the `{ k: Integer }` in the `h in { k: Integer }`
            auto hashPatternNode = down_cast<pm_hash_pattern_node>(node);

            auto prismElements = absl::MakeSpan(hashPatternNode->elements.nodes, hashPatternNode->elements.size);
            auto prismRestNode = hashPatternNode->rest;

            for (auto *prismNode : prismElements) {
                desugarPattern(prismNode);
            }
            if (prismRestNode != nullptr) {
                switch (PM_NODE_TYPE(prismRestNode)) {
                    case PM_ASSOC_SPLAT_NODE: {
                        // MatchRest is a structural pattern component - desugar to Nil
                        break;
                    }
                    case PM_NO_KEYWORDS_PARAMETER_NODE: {
                        break;
                    }
                    default:
                        desugarPattern(prismRestNode);
                }
            }

            // Determine the correct location for the pattern
            // If there's a constant, the pattern location excludes it
            core::LocOffsets patternLoc = location;
            if (hashPatternNode->constant) {
                ENFORCE(hashPatternNode->opening_loc.start && hashPatternNode->closing_loc.end,
                        "Hash pattern without parentheses or square brackets?");

                // The `HashPattern` loc doesn't include the constant, if there is one.
                //     `Point[x: Integer => 1, y: Integer => 2]`
                //           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ HashPattern loc
                //      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ConstPattern loc
                patternLoc = translateLoc(hashPatternNode->opening_loc.start, hashPatternNode->closing_loc.end);
            }

            if (auto *prismConstant = hashPatternNode->constant) {
                // A hash pattern can start with a constant that matches against a specific type,
                // (rather than any value whose `#deconstruct_keys` results are matched by the pattern).
                // E.g. the `Point` in `in Point[x: Integer => 1, y: Integer => 2]`
                auto sorbetConstant = desugar(prismConstant);

                // ConstPattern wrapping the hash pattern - the desugared expression is Nil as it's structural
                return MK::Nil(location);
            }

            return MK::Nil(patternLoc);
        }
        case PM_IMPLICIT_NODE: {
            auto implicitNode = down_cast<pm_implicit_node>(node);
            return desugarPattern(implicitNode->value);
        }
        case PM_IN_NODE: { // An `in` pattern such as in a `case` statement, or as a standalone expression.
            auto inNode = down_cast<pm_in_node>(node);

            auto prismPattern = inNode->pattern;
            ast::ExpressionPtr sorbetGuard;
            auto statements = desugarStatements(inNode->statements);

            if (prismPattern != nullptr &&
                (PM_NODE_TYPE_P(prismPattern, PM_IF_NODE) || PM_NODE_TYPE_P(prismPattern, PM_UNLESS_NODE))) {
                pm_statements_node *conditionalStatements = nullptr;

                if (PM_NODE_TYPE_P(prismPattern, PM_IF_NODE)) {
                    auto ifNode = down_cast<pm_if_node>(prismPattern);
                    conditionalStatements = ifNode->statements;
                    sorbetGuard = desugar(ifNode->predicate);
                } else { // PM_UNLESS_NODE
                    ENFORCE(PM_NODE_TYPE_P(prismPattern, PM_UNLESS_NODE));
                    auto unlessNode = down_cast<pm_unless_node>(prismPattern);
                    conditionalStatements = unlessNode->statements;
                    sorbetGuard = desugar(unlessNode->predicate);
                }

                ENFORCE(
                    conditionalStatements->body.size == 1,
                    "In pattern-matching's `in` clause, a conditional (if/unless) guard must have a single statement.");

                desugarPattern(conditionalStatements->body.nodes[0]);
            } else {
                desugarPattern(prismPattern);
            }

            return MK::EmptyTree();
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // A variable binding in a pattern, like the `head` in `[head, *tail]`
            auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
            auto name = translateConstantName(localVarTargetNode->name);

            // For a match variable, the desugared expression is a local variable reference
            // This represents what the variable will be bound to when the pattern matches
            return MK::Local(location, name);
        }
        case PM_PINNED_EXPRESSION_NODE: { // A "pinned" expression, like `^(1 + 2)` in `in ^(1 + 2)`
            auto pinnedExprNode = down_cast<pm_pinned_expression_node>(node);

            auto expr = desugar(pinnedExprNode->expression);

            return MK::Nil(location);
        }
        case PM_PINNED_VARIABLE_NODE: { // A "pinned" variable, like `^x` in `in ^x`
            auto pinnedVarNode = down_cast<pm_pinned_variable_node>(node);

            auto variable = desugar(pinnedVarNode->variable);

            return MK::Nil(location);
        }
        case PM_SPLAT_NODE: { // A splat, like `*a` in an array pattern
            auto prismSplatNode = down_cast<pm_splat_node>(node);
            auto expr = desugar(prismSplatNode->expression);

            // MatchRest is a structural pattern component with no simple desugared expression
            return MK::Nil(location);
        }
        case PM_SYMBOL_NODE: { // A symbol literal, e.g. `:foo`, or `a:` in `{a: 1}`
            auto symNode = down_cast<pm_symbol_node>(node);

            auto [content, _] = translateSymbol(symNode);

            // The legacy parser has two different locations for symbols.
            if (symNode->opening_loc.start == nullptr) {
                // If it uses quoted syntax, the full symbol is included, with the colon and quotes:
                //     x in { "key": value }
                //            ^^^^^^
                location = translateLoc(symNode->value_loc);
            } else {
                // If it uses short-hand syntax, the content is included, without the colon:
                //     x in {  key: value }
                //             ^^^
                // no-op: leave the whole location as-is.
            }

            return MK::Symbol(location, content);
        }
        default: {
            return desugar(node);
        }
    }
}

tuple<ast::MethodDef::PARAMS_store, ast::InsSeq::STATS_store, core::LocOffsets /* enclosingBlockParamLoc */,
      core::NameRef /* enclosingBlockParamName */>
Translator::desugarParametersNode(pm_parameters_node *paramsNode, core::LocOffsets location,
                                  absl::Span<pm_node_t *> blockLocalVariables) {
    if (paramsNode == nullptr) {
        auto paramsStore = ast::MethodDef::PARAMS_store{};

        // Add in the block-local variables, if any.
        paramsStore.reserve(blockLocalVariables.size());
        for (auto *node : blockLocalVariables) {
            ENFORCE(PM_NODE_TYPE_P(node, PM_BLOCK_LOCAL_VARIABLE_NODE));
            // TODO: move `PM_BLOCK_LOCAL_VARIABLE_NODE` case logic to here
            paramsStore.emplace_back(desugar(node));
        }

        return {move(paramsStore), ast::InsSeq::STATS_store{}, core::LocOffsets::none(), core::Names::blkArg()};
    }

    auto requireds = absl::MakeSpan(paramsNode->requireds.nodes, paramsNode->requireds.size);
    auto optionals = absl::MakeSpan(paramsNode->optionals.nodes, paramsNode->optionals.size);
    auto keywords = absl::MakeSpan(paramsNode->keywords.nodes, paramsNode->keywords.size);
    auto posts = absl::MakeSpan(paramsNode->posts.nodes, paramsNode->posts.size);

    auto restSize = paramsNode->rest == nullptr ? 0 : 1;
    auto kwrestSize = paramsNode->keyword_rest == nullptr ? 0 : 1;
    auto blockSize = paramsNode->block == nullptr ? 0 : 1;

    ast::MethodDef::PARAMS_store paramsStore;
    ast::InsSeq::STATS_store statsStore;

    paramsStore.reserve(requireds.size() + optionals.size() + restSize + posts.size() + keywords.size() + kwrestSize +
                        blockSize + blockLocalVariables.size());

    auto desugarPositionalParam = [this, &paramsStore, &statsStore](auto *n) {
        if (PM_NODE_TYPE_P(n, PM_MULTI_TARGET_NODE)) {
            auto multiTargetNode = down_cast<pm_multi_target_node>(n);

            ENFORCE(multiTargetNode->lparen_loc.start);
            ENFORCE(multiTargetNode->lparen_loc.end);
            ENFORCE(multiTargetNode->rparen_loc.start);
            ENFORCE(multiTargetNode->rparen_loc.end);

            // The legacy parser doesn't usually include the parens in the location of a multi-target node,
            // *except* in a block's parameter list.
            auto mlhsLoc = translateLoc(multiTargetNode->lparen_loc.start, multiTargetNode->rparen_loc.end);

            auto [param, destructureExpr] = desugarMlhsParam(mlhsLoc, multiTargetNode);
            paramsStore.emplace_back(move(param));
            statsStore.emplace_back(move(destructureExpr));
        } else {
            paramsStore.emplace_back(desugar(n));
        }
    };

    for (auto *n : requireds) {
        desugarPositionalParam(n);
    }

    for (auto *n : optionals) {
        desugarPositionalParam(n);
    }

    if (paramsNode->rest != nullptr) {
        paramsStore.emplace_back(desugar(paramsNode->rest));
    }

    for (auto *prismNode : posts) {
        // Valid Ruby can only have `**nil` once in the parameter list, which is modelled with a
        // `NoKeywordsParameterNode` in the `keyword_rest` field.
        // If invalid code tries to use more than one `**nil` (like `def foo(**nil, **nil)`),
        // Prism will report an error, but still place the excess `**nil` nodes in `posts` list (never the others like
        // `requireds` or `optionals`), which we need to skip here.
        if (!PM_NODE_TYPE_P(prismNode, PM_NO_KEYWORDS_PARAMETER_NODE)) {
            paramsStore.emplace_back(desugar(prismNode));
        }
    }

    for (auto *kwarg : keywords) {
        paramsStore.emplace_back(desugar(kwarg));
    }

    bool hasForwardingParameter = false;
    if (auto *prismKwRestNode = paramsNode->keyword_rest) {
        auto loc = translateLoc(prismKwRestNode->location);

        switch (PM_NODE_TYPE(prismKwRestNode)) {
            case PM_KEYWORD_REST_PARAMETER_NODE: // `def foo(**kwargs)`
                paramsStore.emplace_back(desugar(prismKwRestNode));
                // TODO: Inline `case PM_KEYWORD_REST_PARAMETER_NODE` logic here.
                break;
            case PM_FORWARDING_PARAMETER_NODE: { // `def foo(...)`
                hasForwardingParameter = true;

                // Desugar `def foo(m, n, ...)` into:
                // `def foo(m, n, *<fwd-args>, **<fwd-kwargs>, &<fwd-block>)`

                // add `*<fwd-args>`
                paramsStore.emplace_back(MK::RestParam(loc, MK::Local(loc, core::Names::fwdArgs())));

                // add `**<fwd-kwargs>`
                paramsStore.emplace_back(MK::RestParam(loc, MK::KeywordArg(loc, core::Names::fwdKwargs())));

                // add `&<fwd-block>`
                paramsStore.emplace_back(MK::BlockParam(loc, MK::Local(loc, core::Names::fwdBlock())));

                break;
            }
            case PM_NO_KEYWORDS_PARAMETER_NODE: { // `def foo(**nil)`
                // TODO: implement logic for `**nil` args
                break;
            }
            default:
                unreachable("Unexpected keyword_rest node type in Hash pattern.");
        }
    }

    core::LocOffsets enclosingBlockParamLoc = core::LocOffsets::none();
    core::NameRef enclosingBlockParamName;
    if (hasForwardingParameter) {
        // Skip invalid block parameter when there is a forwarding parameter, like `def foo(&, ...)`
        enclosingBlockParamName = core::Names::fwdBlock();
    } else if (auto *prismBlockParam = paramsNode->block) {
        auto blockParamLoc = translateLoc(prismBlockParam->base.location);

        if (auto prismName = prismBlockParam->name; prismName != PM_CONSTANT_ID_UNSET) {
            // A named block parameter, like `def foo(&block)`
            enclosingBlockParamName = translateConstantName(prismName);

            // The location doesn't include the `&`, only the name of the block parameter
            constexpr uint32_t length = "&"sv.size();
            blockParamLoc = core::LocOffsets{blockParamLoc.beginPos() + length, blockParamLoc.endPos()};
        } else { // An anonymous block parameter, like `def foo(&)`
            enclosingBlockParamName = core::Names::ampersand();
        }
        enclosingBlockParamLoc = blockParamLoc;

        auto blockParamExpr = MK::BlockParam(blockParamLoc, MK::Local(blockParamLoc, enclosingBlockParamName));
        paramsStore.emplace_back(move(blockParamExpr));
    } else {
        // Desugaring a method def like `def foo(a, b)` should behave like `def foo(a, b, &<blk>)`,
        // so we set a synthetic name here for `yield` to use.
        enclosingBlockParamName = core::Names::blkArg();
    }

    // Add in the block-local variables, if any.
    for (auto *node : blockLocalVariables) {
        ENFORCE(PM_NODE_TYPE_P(node, PM_BLOCK_LOCAL_VARIABLE_NODE));
        paramsStore.emplace_back(desugar(node));
    }

    return make_tuple(move(paramsStore), move(statsStore), enclosingBlockParamLoc, enclosingBlockParamName);
}

core::LocOffsets Translator::findItParamUsageLoc(pm_statements_node *statements) {
    ENFORCE(statements != nullptr);
    ENFORCE(0 < statements->body.size, "Can never have an ItParam node on a block with no statements.");

    core::LocOffsets result;

    walkPrismAST(up_cast(statements), [this, &result](const pm_node_t *node) -> bool {
        if (PM_NODE_TYPE_P(node, PM_IT_LOCAL_VARIABLE_READ_NODE)) {
            result = this->translateLoc(node->location);
            // Found the first usage, stop walking
            return false;
        }
        // Don't descend into nested blocks/lambdas that have their own 'it' parameter
        if (PM_NODE_TYPE_P(node, PM_BLOCK_NODE)) {
            auto blockNode = down_cast<pm_block_node>(const_cast<pm_node_t *>(node));
            if (blockNode->parameters != nullptr && PM_NODE_TYPE_P(blockNode->parameters, PM_IT_PARAMETERS_NODE)) {
                // This nested block has its own 'it', don't descend into it
                return false;
            }
        }
        if (PM_NODE_TYPE_P(node, PM_LAMBDA_NODE)) {
            auto lambdaNode = down_cast<pm_lambda_node>(const_cast<pm_node_t *>(node));
            if (lambdaNode->parameters != nullptr && PM_NODE_TYPE_P(lambdaNode->parameters, PM_IT_PARAMETERS_NODE)) {
                // This nested lambda has its own 'it', don't descend into it
                return false;
            }
        }
        return true;
    });

    return result;
}

std::array<core::LocOffsets, 9>
Translator::findNumberedParamsUsageLocs(core::LocOffsets loc, pm_statements_node *statements, uint8_t maxParamNumber) {
    ENFORCE(statements != nullptr);
    ENFORCE(0 < statements->body.size, "Can never have a NumParams node on a block with no statements.");

    auto result = std::array<core::LocOffsets, 9>{};

    // The first `maxParamNumber` elements of `result` which we're actually using for this call.
    auto activeRegion = absl::MakeSpan(result).first(maxParamNumber);

    walkPrismAST(up_cast(statements), [this, &activeRegion](const pm_node_t *node) -> bool {
        if (PM_NODE_TYPE_P(node, PM_LOCAL_VARIABLE_READ_NODE)) {
            auto var = down_cast<pm_local_variable_read_node>(const_cast<pm_node_t *>(node));
            auto varName = this->parser.resolveConstant(var->name);

            if (varName.length() == 2 && varName[0] == '_' && '1' <= varName[1] && varName[1] <= '9') {
                auto number = varName[1] - '0';

                // We've already found the first usage of this numbered parameter.
                // Skip it, and keep searching for the rest.
                if (activeRegion[number - 1].exists()) {
                    return true;
                }

                activeRegion[number - 1] = this->translateLoc(node->location);
            }

            if (absl::c_all_of(activeRegion, [](const core::LocOffsets &loc) { return loc.exists(); })) {
                // We've seen all the numbered parameters we needed, so we can stop early.
                return false;
            }
        }
        return true;
    });

    return result;
}

ast::MethodDef::PARAMS_store
Translator::translateNumberedParametersNode(pm_numbered_parameters_node *numberedParamsNode,
                                            pm_statements_node_t *statements) {
    auto location = translateLoc(numberedParamsNode->base.location);

    auto paramCount = numberedParamsNode->maximum;

    ENFORCE(1 <= paramCount, "A `pm_numbered_parameters_node_t` node should have at least one parameter");
    ENFORCE(paramCount <= 9, "Ruby only supports 9 numbered parameters (`_9` but no `_10`).");

    auto numberedParamsUsageLocs = findNumberedParamsUsageLocs(location, statements, paramCount);
    ENFORCE(paramCount <= numberedParamsUsageLocs.size());

    ast::MethodDef::PARAMS_store result;
    for (auto i = 1; i <= paramCount; i++) {
        // Numbered parameters are implicit, so they don't have a real location in the body.
        // However, we need somewhere for the error messages to point to, so we use the
        // location of the first *usage* of this numbered parameter (or none if it was never used).
        auto usageLoc = numberedParamsUsageLocs[i - 1];
        auto name = ctx.state.enterNameUTF8("_" + to_string(i));

        result.emplace_back(MK::Local(usageLoc, name));
    }

    return result;
}

Translator::DesugaredBlockArgument Translator::desugarBlock(pm_node_t *block, pm_arguments_node *otherArgs,
                                                            pm_location_t parentLoc) {
    auto result = DesugaredBlockArgument::none();

    if (block) {
        if (PM_NODE_TYPE_P(block, PM_BLOCK_NODE)) { // a literal block with `{ ... }` or `do ... end`
            auto blockNode = down_cast<pm_block_node>(block);

            result = DesugaredBlockArgument::literalBlock(desugarLiteralBlock(
                blockNode->body, blockNode->parameters, blockNode->base.location, blockNode->opening_loc));
        } else {
            ENFORCE(PM_NODE_TYPE_P(block, PM_BLOCK_ARGUMENT_NODE)); // the `&b` in `a.map(&b)`

            auto *bp = down_cast<pm_block_argument_node>(block);

            result = desugarBlockPassArgument(bp);
        }
    }

    auto hasFwdArgs = otherArgs != nullptr && PM_NODE_FLAG_P(otherArgs, PM_ARGUMENTS_NODE_FLAGS_CONTAINS_FORWARDING);

    if (hasFwdArgs) { // Desugar a call like `foo(...)` so it has a block argument like `foo(..., &b)`.
        ENFORCE(!result.exists(), "The parser should have rejected a call with both a block pass "
                                  "argument and forwarded args (e.g. `foo(&b, ...)`)");

        result = DesugaredBlockArgument::blockPass(MK::Local(translateLoc(parentLoc), core::Names::fwdBlock()),
                                                   core::LocOffsets::none());
    }

    return result;
}

ast::ExpressionPtr Translator::desugarLiteralBlock(pm_node *blockBodyNode, pm_node *blockParameters,
                                                   pm_location_t blockLocation, pm_location_t blockNodeOpeningLoc) {
    auto blockBody = this->enterBlockContext().desugarNullable(blockBodyNode);
    ast::MethodDef::PARAMS_store blockParamsStore;
    auto blockLoc = translateLoc(blockLocation);

    if (blockParameters != nullptr) {
        switch (PM_NODE_TYPE(blockParameters)) {
            case PM_BLOCK_PARAMETERS_NODE: { // The params declared at the top of a PM_BLOCK_NODE
                // Like the `|x|` in `foo { |x| ... }`
                auto paramsNode = down_cast<pm_block_parameters_node>(blockParameters);

                auto paramsLoc = translateLoc(paramsNode->base.location);

                if (paramsNode->parameters) {
                    auto blockLocalVariables = absl::MakeSpan(paramsNode->locals.nodes, paramsNode->locals.size);

                    ast::InsSeq::STATS_store blockStatsStore;

                    std::tie(blockParamsStore, blockStatsStore, std::ignore, std::ignore) =
                        desugarParametersNode(paramsNode->parameters, paramsLoc, blockLocalVariables);

                    if (!blockStatsStore.empty()) {
                        blockBody = MK::InsSeq(blockLoc, move(blockStatsStore), move(blockBody));
                    }
                }

                break;
            }

            case PM_NUMBERED_PARAMETERS_NODE: { // The params in a PM_BLOCK_NODE with numbered params
                // Like the implicit `|_1, _2, _3|` in `foo { _3 }`
                auto numberedParamsNode = down_cast<pm_numbered_parameters_node>(blockParameters);

                // Use a 0-length loc just after the `do` or `{` token, as if you had written:
                //     do|_1, _2| ... end`
                //       ^
                //     {|_1, _2| ... }`
                //      ^

                blockParamsStore =
                    translateNumberedParametersNode(numberedParamsNode, down_cast<pm_statements_node>(blockBodyNode));

                break;
            }

            case PM_IT_PARAMETERS_NODE: { // The 'it' default block parameter, e.g. `a.map { it + 1 }`
                // Use a 0-length loc just after the `do` or `{` token, similar to numbered params
                auto itParamLoc = translateLoc(blockNodeOpeningLoc.end, blockNodeOpeningLoc.end);

                // Find the actual usage location of 'it' in the block body by walking the AST
                auto itUsageLoc = itParamLoc;
                if (blockBodyNode != nullptr) {
                    auto statements = down_cast<pm_statements_node>(blockBodyNode);
                    auto foundLoc = findItParamUsageLoc(statements);
                    if (foundLoc.exists()) {
                        itUsageLoc = foundLoc;
                    }
                }

                // Single 'it' parameter - use the original name (not a unique one)
                // Unlike numbered parameters, 'it' uses the actual name "it" so that
                // local variables named 'it' in the same scope can shadow it
                blockParamsStore.emplace_back(MK::Local(itUsageLoc, core::Names::it()));
                break;
            }

            default: {
                unreachable("Found a {} block parameter type, which is not implemented yet ",
                            pm_node_type_to_str(PM_NODE_TYPE(blockParameters)));
            }
        }
    }

    return MK::Block(blockLoc, move(blockBody), move(blockParamsStore));
}

Translator::DesugaredBlockArgument Translator::desugarBlockPassArgument(pm_block_argument_node *bp) {
    auto blockPassLoc = translateLoc(bp->base.location); // The location of the entire block pass, including the `&`.

    if (bp->expression) { // Block pass with an explicit expression, like `f(&block)`
        if (PM_NODE_TYPE_P(bp->expression, PM_SYMBOL_NODE)) {
            // Symbol proc, e.g. `&:foo` - desugar to a literal block
            auto symbol = down_cast<pm_symbol_node>(bp->expression);
            return DesugaredBlockArgument::literalBlock(desugarSymbolProc(symbol));
        } else {
            return DesugaredBlockArgument::blockPass(desugar(bp->expression), blockPassLoc);
        }
    } else { // Anonymous block pass, like `f(&)`
        // Treat it as a block pass of a local variable literally named `&`.
        auto loc = translateLoc(bp->base.location).copyEndWithZeroLength();
        return DesugaredBlockArgument::blockPass(MK::Local(loc, core::Names::ampersand()), blockPassLoc);
    }
}

// Desugar a Symbol block pass argument (like the `&foo` in `m(&:foo)`) into a block literal.
// `&:foo` => `{ |*temp| (temp[0]).foo(*temp[1, LONG_MAX]) }`
//
// This works because Sorbet is guaranteed to infer a tuple type for `temp` corresponding to however
// many block params the enclosing Send declares (or T.untyped). From there, various tuple-specific
// intrinsics kick in:
//
// - temp[0]            (evaluates to the 0th elem of the tuple)
// - temp[1, LONG_MAX]  (evalutes to a tuple type if temp is a tuple type)
// - foo(*expr)         (call-with-splat handles case of splatted tuple type)
ast::ExpressionPtr Translator::desugarSymbolProc(pm_symbol_node *symbol) {
    auto [symbolName, loc] = translateSymbol(symbol);
    auto loc0 = loc.copyWithZeroLength(); // TODO: shorten name

    // `temp` does not refer to any specific source text, so give it a 0-length Loc so LSP ignores it.
    core::NameRef tempName = nextUniqueDesugarName(core::Names::blockPassTemp());

    // `temp[0]`
    auto recv = MK::Send1(loc0, MK::Local(loc0, tempName), core::Names::squareBrackets(), loc0, MK::Int(loc0, 0));

    // `temp[1, LONG_MAX]`
    auto sliced = MK::Send2(loc0, MK::Local(loc0, tempName), core::Names::squareBrackets(), loc0, MK::Int(loc0, 1),
                            MK::Int(loc0, LONG_MAX));

    // `(temp[0]).foo(*temp[1, LONG_MAX])`
    auto body = MK::CallWithSplat(loc, move(recv), symbolName, loc0, MK::Splat(loc0, move(sliced)));

    // `{ |*temp| (temp[0]).foo(*temp[1, LONG_MAX]) }`
    return MK::Block1(loc, move(body), MK::RestParam(loc0, MK::Local(loc0, tempName)));
}

ast::ExpressionPtr Translator::desugarMethodCall(ast::ExpressionPtr receiver, core::NameRef methodName,
                                                 core::LocOffsets messageLoc, pm_arguments_node *argumentsNode,
                                                 pm_location_t closingLoc, Translator::DesugaredBlockArgument block,
                                                 core::LocOffsets location, bool isPrivateOk) {
    pm_node_t *receiverNode = nullptr; // TODO: Remove me

    ast::Send::Flags flags;
    flags.isPrivateOk = isPrivateOk;

    absl::Span<pm_node_t *> prismArgs;
    if (argumentsNode != nullptr) {
        prismArgs = absl::MakeSpan(argumentsNode->arguments.nodes, argumentsNode->arguments.size);
    }

    // The legacy parser nodes don't include the literal block argument (if any), but the desugar nodes do
    // include it.
    core::LocOffsets sendLoc;  // The location of the "send" node, exluding any literal block, if any.
    core::LocOffsets blockLoc; // The location of just the block node, on its own.
    core::LocOffsets sendWithBlockLoc = location;
    location = core::LocOffsets::none(); // Invalidate this to ensure we don't use it again in this path.
    if (block.exists()) {
        // There's a block, so we need to calculate the location of the "send" node, excluding it.
        // Start with message location joined with receiver location
        auto initialLoc = receiver.loc().join(messageLoc);
        std::tie(sendLoc, blockLoc) = computeMethodCallLoc(initialLoc, receiverNode, prismArgs, closingLoc, block);
    } else {
        // There's no block, so the `sendLoc` and `sendWithBlockLoc` are the same, so we can just skip
        // the finicky logic in `computeMethodCallLoc()`.
        sendLoc = sendWithBlockLoc;
    }
    auto sendLoc0 = sendLoc.copyWithZeroLength();

    if (methodName == core::Names::squareBrackets() || methodName == core::Names::squareBracketsEq()) {
        // Empty funLoc implies that errors should use the callLoc
        messageLoc.endLoc = messageLoc.beginLoc;
    }

    // Translate the args, detecting special cases along the way,
    // that will require the call to be desugared into a magic call.
    //
    // TODO list:
    // * Optimize via `PM_ARGUMENTS_NODE_FLAGS_CONTAINS_SPLAT`
    //   We can skip the `hasFwdRestArg`/`hasSplat` logic below if it's false.
    //   However, we still need the loop if it's true, to be able to tell the two cases apart.

    // true if the call contains a forwarded argument like `foo(...)`
    auto hasFwdArgs =
        argumentsNode != nullptr && PM_NODE_FLAG_P(argumentsNode, PM_ARGUMENTS_NODE_FLAGS_CONTAINS_FORWARDING);
    auto hasFwdRestArg = false; // true if the call contains an anonymous forwarded rest arg like `foo(*)`
    auto hasSplat = false;      // true if the call contains a splatted expression like `foo(*a)`
    pm_keyword_hash_node *kwargsHashNode = nullptr;
    if (!prismArgs.empty()) {
        // Pop the Kwargs Hash off the end of the arguments, if there is one.
        if (PM_NODE_TYPE_P(prismArgs.back(), PM_KEYWORD_HASH_NODE)) {
            auto keywordHashNode = down_cast<pm_keyword_hash_node>(prismArgs.back());
            auto elements = absl::MakeSpan(keywordHashNode->elements.nodes, keywordHashNode->elements.size);

            auto isKwargs = PM_NODE_FLAG_P(keywordHashNode, PM_KEYWORD_HASH_NODE_FLAGS_SYMBOL_KEYS) ||
                            absl::c_all_of(elements, [](auto *node) {
                                if (PM_NODE_TYPE_P(node, PM_ASSOC_NODE)) {
                                    auto pair = down_cast<pm_assoc_node>(node);
                                    return pair->key && PM_NODE_TYPE_P(pair->key, PM_SYMBOL_NODE);
                                }
                                if (PM_NODE_TYPE_P(node, PM_ASSOC_SPLAT_NODE)) {
                                    return true;
                                }
                                return false;
                            });

            if (isKwargs) {
                kwargsHashNode = keywordHashNode;

                // Remove the kwargsHash from the arguments Span, so it's not revisited by the `for` loop below.
                prismArgs.remove_suffix(1);
            }
        }

        // Detect splats in the argument list
        if (PM_NODE_FLAG_P(argumentsNode, PM_ARGUMENTS_NODE_FLAGS_CONTAINS_SPLAT)) {
            for (auto &arg : prismArgs) {
                if (PM_NODE_TYPE_P(arg, PM_SPLAT_NODE)) {
                    auto splatNode = down_cast<pm_splat_node>(arg);
                    if (splatNode->expression == nullptr) { // An anonymous splat like `f(*)`
                        hasFwdRestArg = true;
                    } else { // Splatting an expression like `f(*a)`
                        hasSplat = true;
                    }
                }
            }
        }
    }

    if (ast::isa_tree<ast::EmptyTree>(receiver)) {
        receiver = MK::Self(sendLoc0);
    }

    if (hasSplat || hasFwdRestArg || hasFwdArgs) { // f(*a) || f(*) || f(...)
        // If we have a splat anywhere in the argument list, desugar the argument list as a single Array node,
        // and synthesize a call to `::Magic.<callWithSplat>(receiver, method, argArray[, &blk])`
        // The `callWithSplat` implementation (in C++) will unpack a tuple type and call into the normal
        // call mechanism.

        ast::Array::ENTRY_store argExprs;
        argExprs.reserve(prismArgs.size());
        for (auto *arg : prismArgs) {
            if (PM_NODE_TYPE_P(arg, PM_SPLAT_NODE) && down_cast<pm_splat_node>(arg)->expression == nullptr) {
                continue; // Skip anonymous splats (like `f(*)`), which are handled separately in `PM_CALL_NODE`
            } else if (PM_NODE_TYPE_P(arg, PM_FORWARDING_ARGUMENTS_NODE)) {
                continue; // Skip forwarded args (like `f(...)`), which are handled separately in `PM_CALL_NODE`
            }

            argExprs.emplace_back(desugar(arg));
        }
        auto argsEmpty = argExprs.empty();
        auto argsArrayExpr = desugarArray(sendLoc0, prismArgs, move(argExprs));

        if (hasFwdRestArg) { // f(*)
            auto loc = sendWithBlockLoc;

            // `<fwd-args>`
            auto fwdArgs = MK::Local(loc, core::Names::fwdArgs());

            // `<fwd-args>.to_a()`
            auto argsSplat = MK::Send0(loc, move(fwdArgs), core::Names::toA(), sendLoc0);

            // `T.unsafe(<fwd-args>.to_a())`
            auto tUnsafe = MK::Unsafe(loc, move(argsSplat));

            // `argsArrayExpr.concat(T.unsafe(<fwd-args>.to_a()))`
            auto argsConcat = MK::Send1(loc, move(argsArrayExpr), core::Names::concat(), sendLoc0, move(tUnsafe));

            argsArrayExpr = move(argsConcat);
        } else if (hasFwdArgs) { // f(...)
            auto loc = sendWithBlockLoc;

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //                                       ^^^^^^^^^^
            auto fwdArgs = MK::Local(loc, core::Names::fwdArgs());

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //                       ^^^^^^^^^^^^^^^^          ^
            auto argsSplat = MK::Splat(loc, move(fwdArgs));

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //  ^^^^^^^^^^^^^^^^^^^^^                           ^
            auto argsConcat =
                argsEmpty ? move(argsSplat)
                          : MK::Send1(loc, move(argsArrayExpr), core::Names::concat(), sendLoc0, move(argsSplat));

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //                                                                                    ^^^^^^^^^^^^
            auto fwdKwargs = MK::Local(loc, core::Names::fwdKwargs());

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //                                                            ^^^^^^^^^^^^^^^^^^^^^^^^            ^
            auto kwargsSplat = MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), sendLoc0, move(fwdKwargs));

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //                                                           ^                                     ^
            auto kwargsArray = MK::Array1(loc, move(kwargsSplat));

            // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
            //                                                   ^^^^^^^^ ^
            argsConcat = MK::Send1(loc, move(argsConcat), core::Names::concat(), sendLoc0, move(kwargsArray));

            argsArrayExpr = move(argsConcat);
        }

        // Build up an array that represents the keyword args for the send.
        // When there is a Kwsplat, treat all keyword arguments as a single argument.
        // If the kwargs hash is not present, make a `nil` to put in the place of that argument.
        // This will be used in the implementation of the intrinsic to tell the difference between keyword
        // args, keyword args with kw splats, and no keyword args at all.
        ExpressionPtr kwargsExpr;
        if (kwargsHashNode != nullptr) {
            ast::Array::ENTRY_store kwargElements;
            flattenKwargs(kwargsHashNode, kwargElements);
            ast::desugar::DuplicateHashKeyCheck::checkSendArgs(ctx, 0, kwargElements);

            kwargsExpr = MK::Array(sendWithBlockLoc, move(kwargElements));
        } else {
            kwargsExpr = MK::Nil(sendWithBlockLoc);
        }

        auto numPosArgs = 4;
        ast::Send::ARGS_store magicSendArgs;
        magicSendArgs.reserve(numPosArgs); // TODO: reserve room for a block pass arg
        magicSendArgs.emplace_back(move(receiver));
        magicSendArgs.emplace_back(MK::Symbol(sendLoc0, methodName));
        magicSendArgs.emplace_back(move(argsArrayExpr));
        magicSendArgs.emplace_back(move(kwargsExpr));

        if (block.hasBlockPass()) {
            // Desugar a call with a splat, and any other expression as a block pass argument.
            // E.g. `foo(*splat, &block)`

            auto blockPassLoc = hasFwdArgs ? sendLoc.copyEndWithZeroLength() : block.blockPassLoc;

            magicSendArgs.emplace_back(move(block.blockPassExpr));
            numPosArgs++;

            return MK::Send(sendWithBlockLoc, MK::Magic(blockPassLoc), core::Names::callWithSplatAndBlockPass(),
                            messageLoc, numPosArgs, move(magicSendArgs), flags);
        }

        if (block.hasLiteralBlock()) {
            magicSendArgs.emplace_back(move(block.literalBlockExpr));
            flags.hasBlock = true;
        }

        // Desugar any call with a splat and without a block pass argument.
        // If there's a literal block argument, that's handled here, too.
        // E.g. `foo(*splat)` or `foo(*splat) { |x| puts(x) }`
        return MK::Send(sendWithBlockLoc, MK::Magic(sendWithBlockLoc), core::Names::callWithSplat(), messageLoc,
                        numPosArgs, move(magicSendArgs), flags);
    }

    // Grab a copy of the argument count, before we concat in the kwargs key/value pairs. // huh?
    int numPosArgs = prismArgs.size();

    if (block.hasBlockPass()) {
        // FIXME: move this comment
        // Special handling for non-Symbol block pass args, like `a.map(&block)`
        // Symbol procs like `a.map(:to_s)` are rewritten into literal block arguments,
        // and handled separately below.

        // Desugar a call without a splat, and any other expression as a block pass argument.
        // E.g. `a.each(&block)`

        auto blockPassLoc = block.blockPassLoc;

        ast::Send::ARGS_store magicSendArgs;
        magicSendArgs.reserve(3 + prismArgs.size());
        magicSendArgs.emplace_back(move(receiver));
        magicSendArgs.emplace_back(MK::Symbol(sendLoc0, methodName));
        magicSendArgs.emplace_back(move(block.blockPassExpr));

        numPosArgs += 3;

        for (auto *arg : prismArgs) {
            magicSendArgs.emplace_back(desugar(arg));
        }

        if (kwargsHashNode) {
            flattenKwargs(kwargsHashNode, magicSendArgs);
            ast::desugar::DuplicateHashKeyCheck::checkSendArgs(ctx, numPosArgs, magicSendArgs);
        }

        return MK::Send(sendWithBlockLoc, MK::Magic(blockPassLoc), core::Names::callWithBlockPass(), messageLoc,
                        numPosArgs, move(magicSendArgs), flags);
    }

    ast::Send::ARGS_store sendArgs{};
    // TODO: reserve size for kwargs Hash keys and values, if needed.
    // TODO: reserve size for the block, if needed.
    sendArgs.reserve(prismArgs.size());
    for (auto *arg : prismArgs) {
        sendArgs.emplace_back(desugar(arg));
    }

    if (kwargsHashNode) {
        flattenKwargs(kwargsHashNode, sendArgs);
        ast::desugar::DuplicateHashKeyCheck::checkSendArgs(ctx, numPosArgs, sendArgs);
    }

    if (block.hasLiteralBlock()) {
        sendArgs.emplace_back(move(block.literalBlockExpr));
        flags.hasBlock = true;
    }

    return MK::Send(sendWithBlockLoc, move(receiver), methodName, messageLoc, numPosArgs, move(sendArgs), flags);
}

template <typename StoreType>
StoreType Translator::desugarArguments(pm_arguments_node *argsNode, pm_node *blockArgumentNode) {
    static_assert(std::is_same_v<StoreType, ast::Send::ARGS_store> ||
                      std::is_same_v<StoreType, ast::Array::ENTRY_store>,
                  "desugarArguments can only be used with ast::Send::ARGS_store or ast::Array::ENTRY_store");

    StoreType results;

    absl::Span<pm_node *> prismArgs;

    if (argsNode != nullptr) {
        prismArgs = absl::MakeSpan(argsNode->arguments.nodes, argsNode->arguments.size);
    }

    results.reserve(prismArgs.size() + (blockArgumentNode == nullptr ? 0 : 1));

    for (auto *prismArg : prismArgs) {
        auto expr = desugarNullable(prismArg);
        results.emplace_back(move(expr));
    }
    if (blockArgumentNode != nullptr) {
        auto expr = desugarNullable(blockArgumentNode);
        results.emplace_back(move(expr));
    }

    return results;
}

// Helper function for Break/Next/Return nodes, which all have the same structure:
// - If no arguments, create node with EmptyTree
// - If one argument, create node with that argument
// - If multiple arguments, create node with an Array of arguments
ast::ExpressionPtr Translator::desugarBreakNextReturn(pm_arguments_node *argsNode) {
    if (argsNode == nullptr) {
        return MK::EmptyTree();
    }

    if (argsNode->arguments.size == 1) {
        return desugar(argsNode->arguments.nodes[0]);
    }

    auto arguments = nodeListToStore<ast::Array::ENTRY_store>(argsNode->arguments);

    // Exclude the "return", "break", or "next" keywords from the array location
    auto arrayLoc = arguments.front().loc().join(arguments.back().loc());
    return MK::Array(arrayLoc, move(arguments));
}

ast::ExpressionPtr Translator::desugarArray(core::LocOffsets location, absl::Span<pm_node_t *> prismElements,
                                            ast::Array::ENTRY_store elements) {
    auto locZeroLen = location.copyWithZeroLength();
    auto calledFromCallNode = location.empty();

    ast::Array::ENTRY_store elems;
    elems.reserve(elements.size());

    ExpressionPtr lastMerge;
    ENFORCE(elements.size() <= prismElements.size());
    for (int prismIndex = 0, sorbetIndex = 0; prismIndex < prismElements.size() && sorbetIndex < elements.size();
         prismIndex++, sorbetIndex++) {
        auto *node = prismElements[prismIndex];
        auto &stat = elements[sorbetIndex];

        if (PM_NODE_TYPE_P(node, PM_SPLAT_NODE)) {
            auto isAnonymousSplat = down_cast<pm_splat_node>(node)->expression == nullptr;
            if (calledFromCallNode && isAnonymousSplat) {
                prismIndex++;
                continue; // Skip anonymous splats (like `f(*)`), which are handled separately in `PM_CALL_NODE`
            }

            // Desugar [a, *x, remaining] into a.concat(<splat>(x)).concat(remaining)

            // The Splat was already desugared to Send{Magic.splat(arg)} with the splat's own location.
            // But for array literals, we want the splat to have the array's location to match
            // the legacy parser's behavior (important for error messages and hover).
            auto var = move(stat);

            // The parser::Send case makes a fake parser::Array with locZeroLen to hide callWithSplat
            // methods from hover. Using the array's loc means that we will get a zero-length loc for
            // the Splat in that case, and non-zero if there was a real Array literal.
            if (auto splattedExpr = ast::MK::extractSplattedExpression(var)) {
                // Extract the argument from the old Send and create a new one with array's location
                if (isAnonymousSplat) {
                    // Recreate the splat and local expr with the correct locations
                    var = MK::Splat(location, MK::Local(location, core::Names::star()));
                } else {
                    // Recreate the splat with the correct location, keep the splatted expression as-is.
                    var = MK::Splat(location, move(splattedExpr));
                }
            }

            if (elems.empty()) {
                if (lastMerge != nullptr) {
                    lastMerge = MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(var));
                } else {
                    lastMerge = move(var);
                }
            } else {
                ExpressionPtr current = MK::Array(location, move(elems));
                /* reassign instead of clear to work around https://bugs.llvm.org/show_bug.cgi?id=37553 */
                elems = ast::Array::ENTRY_store();
                if (lastMerge != nullptr) {
                    lastMerge = MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(current));
                } else {
                    lastMerge = move(current);
                }
                lastMerge = MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(var));
            }
        } else {
            elems.emplace_back(move(stat));
        }
    };

    ExpressionPtr res;
    if (elems.empty()) {
        if (lastMerge != nullptr) {
            res = move(lastMerge);
        } else {
            // Empty array
            res = MK::Array(location, move(elems));
        }
    } else {
        res = MK::Array(location, move(elems));
        if (lastMerge != nullptr) {
            res = MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(res));
        }
    }

    return res;
}

ast::ExpressionPtr Translator::desugarKeyValuePairs(core::LocOffsets loc, pm_node_list_t elements) {
    auto kvPairs = absl::MakeSpan(elements.nodes, elements.size);

    auto locZeroLen = loc.copyWithZeroLength();

    ast::InsSeq::STATS_store updateStmts;
    updateStmts.reserve(kvPairs.size());

    auto acc = nextUniqueDesugarName(core::Names::hashTemp());

    ast::desugar::DuplicateHashKeyCheck hashKeyDupes(ctx);
    ast::Send::ARGS_store mergeValues;
    mergeValues.reserve(kvPairs.size() * 2 + 1);
    mergeValues.emplace_back(MK::Local(loc, acc));
    bool havePairsToMerge = false;

    // build a hash literal assuming that the argument follows the same format as `mergeValues`:
    // arg 0: the hash to merge into
    // arg 1: key
    // arg 2: value
    // ...
    // arg n: key
    // arg n+1: value
    auto buildHashLiteral = [loc](ast::Send::ARGS_store &mergeValues) {
        ast::Hash::ENTRY_store keys;
        ast::Hash::ENTRY_store values;

        keys.reserve(mergeValues.size() / 2);
        values.reserve(mergeValues.size() / 2);

        // skip the first positional argument for the accumulator that would have been mutated
        for (auto it = mergeValues.begin() + 1; it != mergeValues.end();) {
            keys.emplace_back(move(*it++));
            values.emplace_back(move(*it++));
        }

        return MK::Hash(loc, move(keys), move(values));
    };

    // Desguar
    //   {**x, a: 'a', **y, remaining}
    // into
    //   acc = <Magic>.<to-hash-dup>(x)
    //   acc = <Magic>.<merge-hash-values>(acc, :a, 'a')
    //   acc = <Magic>.<merge-hash>(acc, <Magic>.<to-hash-nodup>(y))
    //   acc = <Magic>.<merge-hash>(acc, remaining)
    //   acc
    for (auto &pairAsExpression : kvPairs) {
        ENFORCE(pairAsExpression != nullptr);

        if (PM_NODE_TYPE_P(pairAsExpression, PM_ASSOC_NODE)) {
            auto *pair = down_cast<pm_assoc_node>(pairAsExpression);

            ast::ExpressionPtr key;
            if (PM_NODE_TYPE_P(pair->key, PM_SYMBOL_NODE)) { // Special case to modify Symbol locations
                auto symbolNode = down_cast<pm_symbol_node>(pair->key);

                auto [symbolContent, _] = translateSymbol(symbolNode);

                // If the opening location is null, the symbol is used as a key with a colon postfix, like `{ a: 1 }`
                // The legacy parser sometimes includes symbol's colons, othertimes not:
                //
                //     k3 = nil    # The implicit lvar accessed by k3 below
                //     def k4; end # The implicit method called by k4 below
                //
                //             :k1        #  9-12 Regular symbol
                //             ^^^
                //            { k2: 1 }   # 10-12 Key with explicit value
                //              ^^
                //            { k3:   }   # 10-13 Key with implicit lvar value access
                //              ^^^         key symbol loc
                //            { k4:   }   # 10-13 Key with implicit method call
                //              ^^^         key symbol loc
                //              ^^^         Sorbet send node loc
                //              ^^^         Sorbet send node methodLoc (Prism excludes the ':' here)
                //     def demo(k5:); end # 10-13 Keyword parameter
                //              ^^^
                //         call(k6:)      # 10-13 Keyword argument
                //              ^^
                if (symbolNode->opening_loc.start == nullptr) {
                    core::LocOffsets symbolLoc;
                    if (PM_NODE_TYPE_P(pair->value, PM_IMPLICIT_NODE)) {
                        auto implicitNode = down_cast<pm_implicit_node>(pair->value);

                        if (PM_NODE_TYPE_P(implicitNode->value, PM_CALL_NODE)) {
                            auto callNode = down_cast<pm_call_node>(implicitNode->value);

                            // Prism's method_loc excludes the ':' here, but Sorbet's legacy parser includes it.
                            // Not a fan of modifying the Prism tree in-place, but the alternative is much trickier.
                            // TODO: revisit this when we extract a helper function for translating call nodes.
                            ENFORCE(symbolNode->base.location.end[-1] == ':');
                            callNode->message_loc.end = symbolNode->base.location.end;
                        }

                        symbolLoc = translateLoc(symbolNode->base.location);
                    } else {
                        // Drop the trailing colon in the key's location
                        symbolLoc = translateLoc(symbolNode->base.location.start, symbolNode->base.location.end - 1);
                    }

                    key = MK::Symbol(symbolLoc, symbolContent);
                } else {
                    key = desugar(pair->key);
                }
            } else {
                key = desugar(pair->key);
            }
            ENFORCE(key != nullptr);

            hashKeyDupes.check(key);
            mergeValues.emplace_back(move(key));

            auto value = desugar(pair->value);
            mergeValues.emplace_back(move(value));

            havePairsToMerge = true;
            continue;
        }

        ENFORCE(PM_NODE_TYPE_P(pairAsExpression, PM_ASSOC_SPLAT_NODE));
        auto splatNode = down_cast<pm_assoc_splat_node>(pairAsExpression);

        ast::ExpressionPtr expr;
        if (splatNode->value) { // Splatting an expression like `f(**h)`
            expr = desugar(splatNode->value);
        } else { // An anonymous splat like `f(**)`
            expr = MK::Unsafe(loc, MK::Local(loc, core::Names::fwdKwargs()));
        }

        if (havePairsToMerge) {
            havePairsToMerge = false;

            // ensure that there's something to update
            if (updateStmts.empty()) {
                updateStmts.emplace_back(MK::Assign(loc, acc, buildHashLiteral(mergeValues)));
            } else {
                int numPosArgs = mergeValues.size();
                updateStmts.emplace_back(MK::Assign(loc, acc,
                                                    MK::Send(loc, MK::Magic(loc), core::Names::mergeHashValues(),
                                                             locZeroLen, numPosArgs, move(mergeValues))));
            }

            mergeValues.clear();
            mergeValues.emplace_back(MK::Local(loc, acc));
        }

        // If this is the first argument to `<Magic>.<merge-hash>`, it needs to be duplicated as that
        // intrinsic is assumed to mutate its first argument.
        if (updateStmts.empty()) {
            updateStmts.emplace_back(
                MK::Assign(loc, acc, MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), locZeroLen, move(expr))));
        } else {
            updateStmts.emplace_back(MK::Assign(
                loc, acc,
                MK::Send2(loc, MK::Magic(loc), core::Names::mergeHash(), locZeroLen, MK::Local(loc, acc),
                          MK::Send1(loc, MK::Magic(loc), core::Names::toHashNoDup(), locZeroLen, move(expr)))));
        }
    };

    if (havePairsToMerge) {
        // There were only keyword args/values present, so construct a hash literal directly
        if (updateStmts.empty()) {
            return buildHashLiteral(mergeValues);
        }

        // there are already other entries in updateStmts, so append the `merge-hash-values` call and fall
        // through to the rest of the processing
        int numPosArgs = mergeValues.size();
        updateStmts.emplace_back(MK::Assign(
            loc, acc,
            MK::Send(loc, MK::Magic(loc), core::Names::mergeHashValues(), locZeroLen, numPosArgs, move(mergeValues))));
    }

    if (updateStmts.empty()) {
        return MK::Hash0(loc);
    } else {
        return MK::InsSeq(loc, move(updateStmts), MK::Local(loc, acc));
    }
}

// Helper to desugar statements from a clause node (rescue/ensure/else), returning EmptyTree if null or empty.
template <typename ClauseNode> ast::ExpressionPtr Translator::desugarClauseStatements(ClauseNode *clause) {
    if (clause == nullptr || clause->statements == nullptr || clause->statements->body.size == 0) {
        return ast::MK::EmptyTree();
    }
    return desugarStatements(clause->statements);
}

// Helper: Calculate the end position for a RescueCase's location.
// Priority: statements > reference > exceptions > keyword
uint32_t Translator::rescueCaseEndPos(const pm_rescue_node &rescueNode) {
    if (rescueNode.statements != nullptr) {
        return translateLoc(rescueNode.statements->base.location).endPos();
    }
    if (rescueNode.reference != nullptr) {
        return translateLoc(rescueNode.reference->location).endPos();
    }
    if (rescueNode.exceptions.size > 0) {
        auto lastEx = rescueNode.exceptions.nodes[rescueNode.exceptions.size - 1];
        return translateLoc(lastEx->location).endPos();
    }
    return translateLoc(rescueNode.keyword_loc).endPos();
}

// Builds rescue cases from a linked list of pm_rescue_node clauses.
ast::Rescue::RESCUE_CASE_store Translator::desugarRescueCases(pm_rescue_node *firstRescueNode) {
    ast::Rescue::RESCUE_CASE_store rescueCases;

    for (pm_rescue_node *rescueNode = firstRescueNode; rescueNode != nullptr; rescueNode = rescueNode->subsequent) {
        auto resbodyLoc = translateLoc(rescueNode->base.location);
        auto rescueKeywordLoc = translateLoc(rescueNode->keyword_loc);

        // If there's a subsequent rescue clause, we want the previous resbody to end at its actual content,
        // not extend all the way to the subsequent rescue.
        //
        // In Prism, the first `rescue` clause extends all the way to `end`, which would consume any comments
        // between rescue clauses. In Whitequark (WQ), each rescue ends at its own statements/reference/exceptions.
        if (rescueNode->subsequent != nullptr) {
            resbodyLoc = core::LocOffsets{resbodyLoc.beginPos(), rescueCaseEndPos(*rescueNode)};
        }

        // Build exception store from exceptions being rescued
        ast::RescueCase::EXCEPTION_store exceptions;
        auto exceptionsNodes = absl::MakeSpan(rescueNode->exceptions.nodes, rescueNode->exceptions.size);
        for (auto *ex : exceptionsNodes) {
            exceptions.emplace_back(desugar(ex));
        }

        // Desugar the rescue body
        ast::ExpressionPtr rescueBodyExpr = desugarStatements(rescueNode->statements);

        // Handle exception variable (e.g., the `e` in `rescue => e`)
        ast::ExpressionPtr varExpr;
        if (rescueNode->reference != nullptr) {
            auto refExpr = desugar(rescueNode->reference);
            bool isLocal = ast::isa_tree<ast::Local>(refExpr);
            if (!isLocal) {
                if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(refExpr)) {
                    isLocal = ident->kind == ast::UnresolvedIdent::Kind::Local;
                }
            }

            if (isLocal) {
                varExpr = move(refExpr);
            } else {
                // Non-local reference (e.g., @ex, @@ex, $ex) - create temp and wrap body
                auto rescueTemp = nextUniqueDesugarName(core::Names::rescueTemp());
                auto varLoc = refExpr.loc();
                varExpr = ast::MK::Local(varLoc, rescueTemp);

                ast::InsSeq::STATS_store stats;
                stats.emplace_back(ast::MK::Assign(varLoc, move(refExpr), ast::MK::Local(varLoc, rescueTemp)));
                rescueBodyExpr = ast::MK::InsSeq(varLoc, move(stats), move(rescueBodyExpr));
            }
        } else {
            // Bare rescue clause with no variable - create synthetic temp variable
            auto rescueTemp = nextUniqueDesugarName(core::Names::rescueTemp());
            auto syntheticVarLoc = (exceptionsNodes.empty() && ast::isa_tree<ast::EmptyTree>(rescueBodyExpr))
                                       ? rescueKeywordLoc.copyWithZeroLength()
                                       : rescueKeywordLoc;
            varExpr = ast::MK::Local(syntheticVarLoc, rescueTemp);
        }

        rescueCases.emplace_back(
            ast::make_expression<ast::RescueCase>(resbodyLoc, move(exceptions), move(varExpr), move(rescueBodyExpr)));
    }

    return rescueCases;
}

// Helper: Calculate the start position for a Rescue node's location.
// Priority: body statements > else statements > rescue keyword > ensure keyword > begin location
const uint8_t *rescueLocStart(const pm_begin_node &beginNode) {
    if (beginNode.statements != nullptr) {
        return beginNode.statements->base.location.start;
    } else if (beginNode.else_clause != nullptr && beginNode.else_clause->statements != nullptr) {
        return beginNode.else_clause->statements->base.location.start;
    } else if (beginNode.rescue_clause != nullptr) {
        return beginNode.rescue_clause->keyword_loc.start;
    } else if (beginNode.ensure_clause != nullptr) {
        return beginNode.ensure_clause->ensure_keyword_loc.start;
    } else {
        return beginNode.base.location.start;
    }
}

// Helper: Calculate the end position for a Rescue node's location.
// Priority: else > rescue > ensure statements (if present) > body > ensure keyword > begin location
const uint8_t *rescueLocEnd(const pm_begin_node &beginNode) {
    if (beginNode.else_clause != nullptr && beginNode.else_clause->statements != nullptr) {
        return beginNode.else_clause->statements->base.location.end;
    } else if (beginNode.rescue_clause != nullptr) {
        // Find the last rescue clause
        pm_rescue_node *lastRescue = beginNode.rescue_clause;
        while (lastRescue->subsequent != nullptr) {
            lastRescue = lastRescue->subsequent;
        }
        if (lastRescue->statements != nullptr) {
            return lastRescue->statements->base.location.end;
        } else {
            return lastRescue->base.location.end;
        }
    } else if (beginNode.ensure_clause != nullptr && beginNode.ensure_clause->statements != nullptr) {
        // Ensure has statements - include them in location
        return beginNode.ensure_clause->statements->base.location.end;
    } else if (beginNode.statements != nullptr) {
        // Empty ensure - end at body statements
        return beginNode.statements->base.location.end;
    } else if (beginNode.ensure_clause != nullptr) {
        // Empty body with empty ensure - use ensure keyword
        return beginNode.ensure_clause->ensure_keyword_loc.end;
    } else {
        return beginNode.base.location.end;
    }
}

// Desugars a pm_begin_node directly into an ast::ExpressionPtr.
// Returns an ast::Rescue when there are rescue or ensure clauses, otherwise returns an InsSeq (or single expression).
ast::ExpressionPtr Translator::desugarBegin(pm_begin_node *beginNodePtr) {
    if (beginNodePtr == nullptr) {
        return ast::MK::EmptyTree();
    }

    auto &beginNode = *beginNodePtr;
    auto beginLoc = translateLoc(beginNode.base.location);
    auto ensureExpr = desugarClauseStatements(beginNode.ensure_clause);
    auto elseExpr = desugarClauseStatements(beginNode.else_clause);
    auto rescueCases = desugarRescueCases(beginNode.rescue_clause);
    bool hasRescue = !rescueCases.empty();
    bool hasEnsure = beginNode.ensure_clause != nullptr;

    // For the body, use the begin node's location when it's a plain begin block (no rescue/ensure)
    // so the InsSeq spans the whole begin...end block
    auto bodyLoc = (hasRescue || hasEnsure) ? core::LocOffsets::none() : beginLoc;
    auto bodyExpr = desugarStatements(beginNode.statements, true, bodyLoc);

    if (hasRescue || hasEnsure) {
        auto loc = translateLoc(rescueLocStart(beginNode), rescueLocEnd(beginNode));
        return ast::make_expression<ast::Rescue>(loc, move(bodyExpr), move(rescueCases), move(elseExpr),
                                                 move(ensureExpr));
    }

    // No rescue or ensure - just return the body (already an InsSeq or single expression)
    // If body is empty, return Nil with the begin node's location
    if (ast::isa_tree<ast::EmptyTree>(bodyExpr)) {
        return ast::MK::Nil(beginLoc);
    }
    return bodyExpr;
}

// Translates the given Prism Statements Node into a `parser::Begin` node or an inlined `parser::Node`.
// @param inlineIfSingle If enabled and there's 1 child node, we skip the `Begin` and just return the one `parser::Node`
// @param overrideLocation If provided, use this location for the Begin node instead of the statements node location
ast::ExpressionPtr Translator::desugarStatements(pm_statements_node *stmtsNode, bool inlineIfSingle,
                                                 core::LocOffsets overrideLocation) {
    if (stmtsNode == nullptr || stmtsNode->body.size == 0) {
        return ast::MK::EmptyTree();
    }

    // For a single statement, do not create a `Begin` node and just return the statement, if that's enabled.
    if (inlineIfSingle && stmtsNode->body.size == 1) {
        return desugarNullable(stmtsNode->body.nodes[0]);
    }

    core::LocOffsets beginNodeLoc;
    if (overrideLocation.exists()) {
        beginNodeLoc = overrideLocation;
    } else {
        auto prismStatements = absl::MakeSpan(stmtsNode->body.nodes, stmtsNode->body.size);

        // Cover the locations spanned from the first to the last statements.
        beginNodeLoc = translateLoc(prismStatements.front()->location.start, prismStatements.back()->location.end);
    }

    auto statements = nodeListToStore<ast::InsSeq::STATS_store>(stmtsNode->body);

    auto finalExpr = move(statements.back());
    statements.pop_back();

    auto instructionSequence = MK::InsSeq(beginNodeLoc, move(statements), move(finalExpr));
    return instructionSequence;
}

// Handles any one of the Prism nodes that models any kind of constant or constant path.
//
// Dynamic constant assignment inside of a method definition will raise a SyntaxError at runtime. In the
// Sorbet validator, there is a check that will crash Sorbet if this is detected statically.
// To work around this, we substitute dynamic constant assignments with a write to a fake local variable
// called `dynamicConstAssign`.
//
// The only exception is that dynamic constant path *operator* assignments inside of a method definition
// do not raise a SyntaxError at runtime, so we want to skip the workaround in that case.
// However, within this method, both regular constant path assignments and constant path operator assignments
// are passed in as `pm_constant_path_node` types, so we need an extra boolean flag to know when to skip the
// workaround.
//
// Usually returns the `SorbetLHSNode`, but for constant writes and targets,
// it can can return an `LVarLhs` as a workaround in the case of a dynamic constant assignment.
template <typename PrismLhsNode, bool checkForDynamicConstAssign>
ast::ExpressionPtr Translator::translateConst(pm_node_t *anyNode) {
    auto node = down_cast<PrismLhsNode>(anyNode);

    // Constant name might be unset, e.g. `::`.
    if (node->name == PM_CONSTANT_ID_UNSET) {
        auto location = translateLoc(node->base.location);
        return MK::UnresolvedConstant(location, MK::EmptyTree(), core::Names::empty());
    }

    // It's important that in all branches `enterNameUTF8` is called, which `translateConstantName` does,
    // so that the name is available for the rest of the pipeline.
    auto name = translateConstantName(node->name);

    if constexpr (checkForDynamicConstAssign) {
        if (this->isInMethodDef()) {
            core::LocOffsets location;
            if constexpr (is_same_v<PrismLhsNode, pm_constant_write_node> ||
                          is_same_v<PrismLhsNode, pm_constant_operator_write_node> ||
                          is_same_v<PrismLhsNode, pm_constant_and_write_node> ||
                          is_same_v<PrismLhsNode, pm_constant_or_write_node>) {
                location = translateLoc(node->name_loc);
            } else if constexpr (is_same_v<PrismLhsNode, pm_constant_path_node>) {
                location = translateLoc(node->base.location);
            } else {
                static_assert(always_false_v<PrismLhsNode>, "Unexpected case");
            }

            // Check if this is a dynamic constant assignment (SyntaxError at runtime)
            // This is a copy of a workaround from `Desugar.cc`, which substitues in a fake assignment,
            // so the parsing can continue. See other usages of `dynamicConstAssign` for more details.
            return MK::Local(location, core::Names::dynamicConstAssign());
        }
    }

    auto location = translateLoc(node->base.location);
    auto constantName = ctx.state.enterNameConstant(name);

    auto constexpr isConstantPath = is_same_v<PrismLhsNode, pm_constant_path_target_node> ||
                                    is_same_v<PrismLhsNode, pm_constant_path_write_node> ||
                                    is_same_v<PrismLhsNode, pm_constant_path_node>;

    ast::ExpressionPtr parentExpr;

    if constexpr (isConstantPath) { // Handle constant paths, has a parent node that needs translation.
        if (auto *prismParentNode = node->parent) {
            // This constant reference is chained onto another constant reference.
            // E.g. given `A::B::C`, if `node` is pointing to the root, `A::B` is the `parent`, and `C` is the
            // `name`.
            //   A::B::C
            //    /    \
            //  A::B   ::C
            //  /  \
            // A   ::B
            parentExpr = desugarNullable(prismParentNode);
        } else { // This is the root of a fully qualified constant reference, like `::A`.
            auto delimiterLoc = translateLoc(node->delimiter_loc); // The location of the `::`
            parentExpr = MK::Constant(delimiterLoc, core::Symbols::root());
        }
    } else { // Handle plain constants like `A`, that aren't part of a constant path.
        static_assert(
            is_same_v<PrismLhsNode, pm_constant_and_write_node> || is_same_v<PrismLhsNode, pm_constant_or_write_node> ||
            is_same_v<PrismLhsNode, pm_constant_operator_write_node> ||
            is_same_v<PrismLhsNode, pm_constant_target_node> || is_same_v<PrismLhsNode, pm_constant_read_node> ||
            is_same_v<PrismLhsNode, pm_constant_write_node>);

        // For writes, location should only include the name, like `FOO` in `FOO = 1`.
        if constexpr (is_same_v<PrismLhsNode, pm_constant_and_write_node> ||
                      is_same_v<PrismLhsNode, pm_constant_or_write_node> ||
                      is_same_v<PrismLhsNode, pm_constant_operator_write_node> ||
                      is_same_v<PrismLhsNode, pm_constant_write_node>) {
            location = translateLoc(node->name_loc);
        }
        parentExpr = MK::EmptyTree();
    }

    return MK::UnresolvedConstant(location, move(parentExpr), constantName);
}

core::NameRef Translator::translateConstantName(pm_constant_id_t constant_id) {
    return ctx.state.enterNameUTF8(parser.resolveConstant(constant_id));
}

core::NameRef Translator::nextUniqueParserName(core::NameRef original) {
    return ctx.state.freshNameUnique(core::UniqueNameKind::Parser, original, ++parserUniqueCounter);
}

core::NameRef Translator::nextUniqueDesugarName(core::NameRef original) {
    return ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, original, ++desugarUniqueCounter);
}

// Translate the options from a Regexp literal, if any. E.g. the `i` in `/foo/i`
ast::ExpressionPtr Translator::desugarRegexpOptions(pm_location_t closingLoc) {
    ENFORCE(closingLoc.start && closingLoc.end);

    auto prismLoc = closingLoc;

    // Chop off Regexp's closing delimiter from the start of the Regopt location,
    // so the location only spans the options themselves:
    //     /foo/im
    //          ^^
    //     $r(foo)im
    //            ^^
    //     $r[foo]im
    //            ^^
    //     $r{foo}im
    //            ^^
    if (closingLoc.start < closingLoc.end) {
        prismLoc.start = closingLoc.start + 1;
    }
    auto location = translateLoc(prismLoc);

    string_view options = sliceLocation(prismLoc);

    // Desugar options to integer flags
    int flags = 0;
    for (auto &chr : options) {
        switch (chr) {
            case 'i':
                flags |= 1; // Regexp::IGNORECASE
                break;
            case 'x':
                flags |= 2; // Regexp::EXTENDED
                break;
            case 'm':
                flags |= 4; // Regexp::MULTILINE
                break;
            default:
                // Encoding options (n, e, s, u) are handled by the parser
                break;
        }
    }
    return MK::Int(location, flags);
}

// Translate an unescaped string from a Regexp literal
ast::ExpressionPtr Translator::desugarRegexp(core::LocOffsets location, core::LocOffsets contentLoc,
                                             pm_string_t content, pm_location_t closingLoc) {
    auto source = parser.extractString(&content);

    auto stringContent = source.empty() ? core::Names::empty() : ctx.state.enterNameUTF8(source);
    // Use full location for empty regexps, to match original parser.
    auto patternLoc = contentLoc.empty() ? location : contentLoc;
    auto pattern = MK::String(patternLoc, stringContent);

    auto options = desugarRegexpOptions(closingLoc);

    auto cnst = MK::Constant(location, core::Symbols::Regexp());

    // Desugar `/ foo / i` to `::Regexp.new("foo", option_flags_int)`
    return MK::Send2(location, move(cnst), core::Names::new_(), location.copyWithZeroLength(), move(pattern),
                     move(options));
}

string_view Translator::sliceLocation(pm_location_t loc) const {
    return cast_prism_string(loc.start, loc.end - loc.start);
}

// Desugar a class, singleton class or module body.
// The body can be a Begin node comprising multiple statements, or a single statement.
ast::ClassDef::RHS_store Translator::desugarClassOrModule(pm_node *prismBodyNode) {
    if (prismBodyNode == nullptr) { // Empty body
        ast::ClassDef::RHS_store result;
        result.emplace_back(MK::EmptyTree());
        return result;
    }

    ENFORCE(PM_NODE_TYPE_P(prismBodyNode, PM_STATEMENTS_NODE));

    auto body = desugar(prismBodyNode);

    if (1 < down_cast<pm_statements_node>(prismBodyNode)->body.size) { // Handle multi-statement body
        auto insSeqExpr = ast::cast_tree<ast::InsSeq>(body);
        ENFORCE(insSeqExpr != nullptr, "The cached expr on every multi-statement Begin should be an InsSeq.")

        ast::ClassDef::RHS_store result;
        result.reserve(insSeqExpr->stats.size());
        for (auto &statement : insSeqExpr->stats) {
            result.emplace_back(move(statement));
        }
        // Move the the final expression too, which is separated out in the `ast::InsSeq`.
        result.emplace_back(move(insSeqExpr->expr));
        return result;
    } else { // Handle single-statement body
        ast::ClassDef::RHS_store result;
        result.emplace_back(move(body));
        return result;
    }
}

core::NameRef Translator::maybeTypedSuper() const {
    bool typedSuper = ctx.state.cacheSensitiveOptions.typedSuper;
    bool shouldUseTypedSuper = typedSuper && !isInAnyBlock && !isInModule;

    return shouldUseTypedSuper ? core::Names::super() : core::Names::untypedSuper();
}

// Context management methods
bool Translator::isInMethodDef() const {
    ENFORCE(enclosingMethodLoc.exists() == enclosingMethodName.exists(),
            "The enclosing method name and location should always both be present, "
            "or both be absecent, depending on whether we're in a method def or not.")

    return enclosingMethodName.exists();
}

Translator Translator::enterMethodDef(bool isSingletonMethod, core::LocOffsets methodLoc, core::NameRef methodName,
                                      core::LocOffsets &enclosingBlockParamLoc,
                                      core::NameRef &enclosingBlockParamName) const {
    auto resetDesugarUniqueCounter = true;
    auto isInModule = this->isInModule && !isSingletonMethod;
    return Translator(*this, resetDesugarUniqueCounter, methodLoc, methodName, enclosingBlockParamLoc,
                      enclosingBlockParamName, isInModule, this->isInAnyBlock);
}

Translator Translator::enterBlockContext() const {
    auto resetDesugarUniqueCounter = false; // Blocks inherit their parent's numbering
    auto isInAnyBlock = true;
    return Translator(*this, resetDesugarUniqueCounter, this->enclosingMethodLoc, this->enclosingMethodName,
                      this->enclosingBlockParamLoc, this->enclosingBlockParamName, this->isInModule, isInAnyBlock);
}

Translator Translator::enterModuleContext(core::LocOffsets &enclosingBlockParamLoc,
                                          core::NameRef &enclosingBlockParamName) const {
    auto resetDesugarUniqueCounter = true;
    auto isInModule = true;
    auto isInAnyBlock = false; // Blocks never persist across a class/module boundary
    return Translator(*this, resetDesugarUniqueCounter, this->enclosingMethodLoc, this->enclosingMethodName,
                      enclosingBlockParamLoc, enclosingBlockParamName, isInModule, isInAnyBlock);
}

Translator Translator::enterClassContext(core::LocOffsets &enclosingBlockParamLoc,
                                         core::NameRef &enclosingBlockParamName) const {
    auto resetDesugarUniqueCounter = true;
    auto isInModule = false;
    auto isInAnyBlock = false; // Blocks never persist across a class/module boundary
    return Translator(*this, resetDesugarUniqueCounter, this->enclosingMethodLoc, this->enclosingMethodName,
                      enclosingBlockParamLoc, enclosingBlockParamName, isInModule, isInAnyBlock);
}

void Translator::reportError(core::LocOffsets loc, const string &message) const {
    auto errorLoc = core::Loc(ctx.file, loc);
    if (auto e = ctx.state.beginError(errorLoc, core::errors::Parser::ParserError)) {
        e.setHeader("{}", message);
    }
}
}; // namespace sorbet::parser::Prism
