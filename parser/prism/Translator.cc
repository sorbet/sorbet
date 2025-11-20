#include "Translator.h"
#include "Helpers.h"

#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/desugar/DuplicateHashKeyCheck.h"
#include "core/errors/desugar.h"

#include "absl/strings/str_replace.h"

// TODO: Clean up after Prism work is done. https://github.com/sorbet/sorbet/issues/9065
#include "common/sort/sort.h"

template class std::unique_ptr<sorbet::parser::Node>;

using namespace std;

namespace sorbet::parser::Prism {

using namespace std::literals::string_view_literals;
using sorbet::ast::MK;
using ExpressionPtr = sorbet::ast::ExpressionPtr;

bool hasExpr(const std::unique_ptr<parser::Node> &node) {
    return node == nullptr || node->hasDesugaredExpr();
}

bool hasExpr(const parser::NodeVec &nodes) {
    return absl::c_all_of(nodes, [](const auto &node) { return hasExpr(node); });
}

template <typename... Tail> bool hasExpr(const std::unique_ptr<parser::Node> &head, const Tail &...tail) {
    return hasExpr(head) && hasExpr(tail...);
}

// Helper template to convert nodes to any store type with takeDesugaredExpr or EmptyTree for nulls.
// This is used to convert a NodeVec to the store type argument for nodes including `Send`, `InsSeq`.
template <typename StoreType> StoreType nodeVecToStore(const sorbet::parser::NodeVec &nodes) {
    StoreType store;
    store.reserve(nodes.size());
    for (const auto &node : nodes) {
        store.emplace_back(node ? node->takeDesugaredExpr() : sorbet::ast::MK::EmptyTree());
    }
    return store;
}

// Collect pattern variable assignments from a pattern node (similar to desugarPatternMatchingVars in PrismDesugar.cc)
static void collectPatternMatchingVars(ast::InsSeq::STATS_store &vars, parser::Node *node) {
    if (auto *var = parser::NodeWithExpr::cast_node<parser::MatchVar>(node)) {
        auto loc = var->loc;
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, var->name, move(val)));
    } else if (auto *rest = parser::NodeWithExpr::cast_node<parser::MatchRest>(node)) {
        collectPatternMatchingVars(vars, rest->var.get());
    } else if (auto *pair = parser::NodeWithExpr::cast_node<parser::Pair>(node)) {
        collectPatternMatchingVars(vars, pair->value.get());
    } else if (auto *as_pattern = parser::NodeWithExpr::cast_node<parser::MatchAs>(node)) {
        auto loc = as_pattern->as->loc;
        auto name = parser::NodeWithExpr::cast_node<parser::MatchVar>(as_pattern->as.get())->name;
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, name, move(val)));
        collectPatternMatchingVars(vars, as_pattern->value.get());
    } else if (auto *array_pattern = parser::NodeWithExpr::cast_node<parser::ArrayPattern>(node)) {
        for (auto &elt : array_pattern->elts) {
            collectPatternMatchingVars(vars, elt.get());
        }
    } else if (auto *array_pattern = parser::NodeWithExpr::cast_node<parser::ArrayPatternWithTail>(node)) {
        for (auto &elt : array_pattern->elts) {
            collectPatternMatchingVars(vars, elt.get());
        }
    } else if (auto *hash_pattern = parser::NodeWithExpr::cast_node<parser::HashPattern>(node)) {
        for (auto &elt : hash_pattern->pairs) {
            collectPatternMatchingVars(vars, elt.get());
        }
    } else if (auto *alt_pattern = parser::NodeWithExpr::cast_node<parser::MatchAlt>(node)) {
        collectPatternMatchingVars(vars, alt_pattern->left.get());
        collectPatternMatchingVars(vars, alt_pattern->right.get());
    }
}

// Allocates a new `NodeWithExpr` with a pre-computed `ExpressionPtr` AST.
template <typename SorbetNode, typename... TArgs>
unique_ptr<parser::Node> Translator::make_node_with_expr(ast::ExpressionPtr desugaredExpr, TArgs &&...args) const {
    auto whiteQuarkNode = make_unique<SorbetNode>(std::forward<TArgs>(args)...);
    if (directlyDesugar) {
        return make_unique<NodeWithExpr>(move(whiteQuarkNode), move(desugaredExpr));
    } else {
        return whiteQuarkNode;
    }
}

// Like `make_node_with_expr`, but specifically for unsupported nodes.
template <typename SorbetNode, typename... TArgs>
std::unique_ptr<parser::Node> Translator::make_unsupported_node(TArgs &&...args) const {
    auto whiteQuarkNode = make_unique<SorbetNode>(std::forward<TArgs>(args)...);
    if (directlyDesugar) {
        if (auto e = ctx.beginIndexerError(whiteQuarkNode->loc, core::errors::Desugar::UnsupportedNode)) {
            e.setHeader("Unsupported node type `{}`", whiteQuarkNode->nodeName());
        }

        return make_unique<NodeWithExpr>(move(whiteQuarkNode), MK::EmptyTree());
    } else {
        return whiteQuarkNode;
    }
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
// If Kwargs Hash contains any splats, we skip the flattening and append the hash as-is.
template <typename Container> void flattenKwargs(unique_ptr<parser::Hash> &kwargsHash, Container &destination) {
    ENFORCE(kwargsHash != nullptr);

    // Skip inlining the kwargs if there are any kwsplat nodes present
    if (absl::c_any_of(kwargsHash->pairs, [](auto &node) {
            // the parser guarantees that if we see a kwargs hash it only contains pair,
            // kwsplat, or forwarded kwrest arg nodes
            ENFORCE(parser::NodeWithExpr::isa_node<parser::Kwsplat>(node.get()) ||
                    parser::NodeWithExpr::isa_node<parser::Pair>(node.get()) ||
                    parser::NodeWithExpr::isa_node<parser::ForwardedKwrestArg>(node.get()));

            return parser::NodeWithExpr::isa_node<parser::Kwsplat>(node.get()) ||
                   parser::NodeWithExpr::isa_node<parser::ForwardedKwrestArg>(node.get());
        })) {
        ENFORCE(kwargsHash->hasDesugaredExpr());
        destination.emplace_back(kwargsHash->takeDesugaredExpr());
        return;
    }

    // Flatten the key/value pairs into the destination
    for (auto &entry : kwargsHash->pairs) {
        if (auto *pair = parser::NodeWithExpr::cast_node<parser::Pair>(entry.get())) {
            ENFORCE(pair->key->hasDesugaredExpr());
            ENFORCE(pair->value->hasDesugaredExpr());
            destination.emplace_back(pair->key->takeDesugaredExpr());
            destination.emplace_back(pair->value->takeDesugaredExpr());
        } else {
            Exception::raise("Unhandled case");
        }
    }

    return;
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

const uint8_t *startLoc(pm_node_t *anyNode);
const uint8_t *endLoc(pm_node_t *anyNode);

// Given a `pm_multi_target_node` or `pm_multi_write_node`, return the location of the left-hand side.
// Conceptually, the location spans from the start of the first element, to the end of the last element.
// Determining the first/last elements is tricky, because they're split across the `lefts`, `rest`, and `rights` fields.
// We could smush them all into a temporary array, but this implementation opts to go for an allocation-free approach.
template <typename PrismNode> pm_location_t mlhsLocation(PrismNode *node) {
    static_assert(
        is_same_v<PrismNode, pm_multi_target_node> || is_same_v<PrismNode, pm_multi_write_node>,
        "Translator::translateMultiTarget can only be used for PM_MULTI_TARGET_NODE and PM_MULTI_WRITE_NODE.");

    auto lefts = absl::MakeSpan(node->lefts.nodes, node->lefts.size);
    auto *middle = node->rest;
    auto rights = absl::MakeSpan(node->rights.nodes, node->rights.size);

    pm_node_t *left = nullptr;
    pm_node_t *right = nullptr;

    if (!lefts.empty()) {
        left = lefts.front();

        // Look for the last element, from right-to-left.
        if (!rights.empty()) {
            right = rights.back();
        } else if (middle && !PM_NODE_TYPE_P(middle, PM_IMPLICIT_REST_NODE)) {
            // Special case: implicit rest nodes (`,`) should not be included in the location:
            //     a, = 1, 2
            //     ^

            right = middle;
        } else {
            right = lefts.back();
        }
    } else if (middle) {
        left = middle;

        // Look for the last element, from right-to-left.
        if (!rights.empty()) {
            right = rights.back();
        } else {
            right = middle;
        }
    } else if (!rights.empty()) {
        left = rights.front();
        right = rights.back();
    } else {
        unreachable("This multi-write node has no lefts, middle, or rights?!");
    }

    ENFORCE(left != nullptr && right != nullptr);

    const uint8_t *leftStartLoc = startLoc(left);
    const uint8_t *rightEndLoc = endLoc(right);
    ENFORCE(leftStartLoc != nullptr && rightEndLoc != nullptr);

    return (pm_location_t){.start = leftStartLoc, .end = rightEndLoc};
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
        auto parserNode = translate(prismNode);
        auto expr = parserNode->takeDesugaredExpr();
        ENFORCE(expr != nullptr, "All arguments must have a desugared expression by now, failed on {}",
                ctx.file.data(ctx).path());

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
ast::ExpressionPtr Translator::desugarMlhs(core::LocOffsets loc, parser::Mlhs *lhs, ast::ExpressionPtr rhs) {
    ast::InsSeq::STATS_store stats;

    core::NameRef tempRhs = nextUniqueDesugarName(core::Names::assignTemp());
    core::NameRef tempExpanded = nextUniqueDesugarName(core::Names::assignTemp());

    int i = 0;
    int before = 0, after = 0;
    bool didSplat = false;
    auto zloc = loc.copyWithZeroLength();

    for (auto &c : lhs->exprs) {
        if (auto *splat = parser::NodeWithExpr::cast_node<parser::SplatLhs>(c.get())) {
            ENFORCE(!didSplat, "did splat already");
            didSplat = true;

            ast::ExpressionPtr lh = splat->takeDesugaredExpr();

            int left = i;
            int right = lhs->exprs.size() - left - 1;

            if (!ast::isa_tree<ast::EmptyTree>(lh)) {
                if (right == 0) {
                    right = 1;
                }
                auto lhloc = lh.loc();
                auto zlhloc = lhloc.copyWithZeroLength();
                // Calling `to_ary` is not faithful to the runtime behavior,
                // but that it is faithful to the expected static type-checking behavior.
                auto ary = MK::Send0(loc, MK::Local(loc, tempExpanded), core::Names::toAry(), zlhloc);
                stats.emplace_back(MK::Assign(lhloc, move(lh), move(ary)));
            }
            i = -right;
        } else {
            if (didSplat) {
                ++after;
            } else {
                ++before;
            }

            auto zcloc = c->loc.copyWithZeroLength();
            auto val =
                MK::Send1(zcloc, MK::Local(zcloc, tempExpanded), core::Names::squareBrackets(), zloc, MK::Int(zloc, i));

            if (auto *mlhs = parser::NodeWithExpr::cast_node<parser::Mlhs>(c.get())) {
                stats.emplace_back(desugarMlhs(mlhs->loc, mlhs, move(val)));
            } else {
                ast::ExpressionPtr lh = c->takeDesugaredExpr();
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
    }

    auto expanded = MK::Send3(loc, MK::Magic(loc), core::Names::expandSplat(), zloc, MK::Local(loc, tempRhs),
                              MK::Int(loc, before), MK::Int(loc, after));
    stats.insert(stats.begin(), MK::Assign(loc, tempExpanded, move(expanded)));
    stats.insert(stats.begin(), MK::Assign(loc, tempRhs, move(rhs)));

    // Regardless of how we destructure an assignment, Ruby evaluates the expression to the entire right hand side,
    // not any individual component of the destructured assignment.
    return MK::InsSeq(loc, move(stats), MK::Local(loc, tempRhs));
}

// Helper to check if an ExpressionPtr represents a T.let call
ast::Send *asTLet(ExpressionPtr &arg) {
    auto send = ast::cast_tree<ast::Send>(arg);
    if (send == nullptr || send->fun != core::Names::let() || send->numPosArgs() < 2) {
        return nullptr;
    }

    if (!ast::MK::isT(send->recv)) {
        return nullptr;
    }

    return send;
}

// Had to widen the type from `parser::Assign` to `parser::Node` to handle `make_node_with_expr` correctly.
// TODO: narrow the type back after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
template <typename PrismAssignmentNode, typename SorbetLHSNode>
unique_ptr<parser::Node> Translator::translateAssignment(pm_node_t *untypedNode) {
    auto node = down_cast<PrismAssignmentNode>(untypedNode);
    auto location = translateLoc(untypedNode->location);
    auto rhs = translate(node->value);

    unique_ptr<parser::Node> lhs;
    if constexpr (is_same_v<PrismAssignmentNode, pm_constant_write_node>) {
        // Handle regular assignment to a "plain" constant, like `A = 1`
        constexpr bool checkForDynamicConstAssign = true;
        lhs = translateConst<pm_constant_write_node, parser::ConstLhs, checkForDynamicConstAssign>(node);
    } else if constexpr (is_same_v<PrismAssignmentNode, pm_constant_path_write_node>) {
        // Handle regular assignment to a constant path, like `A::B::C = 1` or `::C = 1`
        constexpr bool checkForDynamicConstAssign = true;
        auto target = node->target;
        lhs = translateConst<pm_constant_path_node, parser::ConstLhs, checkForDynamicConstAssign>(target);
    } else {
        // Handle regular assignment to any other kind of LHS.
        auto name = translateConstantName(node->name);
        auto loc = translateLoc(node->name_loc);
        auto kind = getIdentKind<SorbetLHSNode>();

        auto expr = ast::make_expression<ast::UnresolvedIdent>(loc, kind, name);
        lhs = make_node_with_expr<SorbetLHSNode>(move(expr), loc, name);
    }

    if (!hasExpr(lhs, rhs)) {
        return make_unique<parser::Assign>(location, move(lhs), move(rhs));
    }

    auto exp = MK::Assign(location, lhs->takeDesugaredExpr(), rhs->takeDesugaredExpr());
    return make_node_with_expr<parser::Assign>(move(exp), location, move(lhs), move(rhs));
}

// widen the type from `parser::OpAsgn` to `parser::Node` to handle `make_node_with_expr` correctly.
// TODO: narrow the type back after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
// The location is the location of the whole Prism assignment node.
template <typename PrismAssignmentNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
unique_ptr<parser::Node> Translator::translateAnyOpAssignment(PrismAssignmentNode *node, core::LocOffsets location,
                                                              unique_ptr<parser::Node> lhs) {
    auto rhs = translate(node->value);

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
unique_ptr<parser::Node> Translator::translateIndexAssignment(pm_node_t *untypedNode, core::LocOffsets location) {
    auto node = down_cast<PrismAssignmentNode>(untypedNode);

    // Handle operator assignment to an indexed expression, like `a[0] += 1`
    auto openingLoc = translateLoc(node->opening_loc);
    auto lBracketLoc = core::LocOffsets{openingLoc.beginLoc, openingLoc.endLoc - 1};

    auto receiver = translate(node->receiver);
    auto args = translateArguments(node->arguments, up_cast(node->block));

    // The LHS location includes the receiver and the `[]`, but not the `=` or rhs.
    // self.example[k] = v
    // ^^^^^^^^^^^^^^^
    auto lhsLoc = translateLoc(node->receiver->location.start, node->closing_loc.end);

    unique_ptr<parser::Node> lhs;
    if (!directlyDesugar || !hasExpr(receiver) || !hasExpr(args)) {
        lhs = make_unique<parser::Send>(lhsLoc, move(receiver), core::Names::squareBrackets(), lBracketLoc, move(args));
    } else {
        auto receiverExpr = receiver->takeDesugaredExpr();
        auto args2 = nodeVecToStore<ast::Send::ARGS_store>(args);

        // Desugar `x[i] = y, z` to `x.[]=(i, y, z)`
        auto send =
            MK::Send(lhsLoc, move(receiverExpr), core::Names::squareBrackets(), lBracketLoc, args.size(), move(args2));
        lhs = make_node_with_expr<parser::Send>(move(send), lhsLoc, move(receiver), core::Names::squareBrackets(),
                                                lBracketLoc, move(args));
    }

    return translateAnyOpAssignment<PrismAssignmentNode, SorbetAssignmentNode, void>(node, location, move(lhs));
}

// The location is the location of the whole Prism assignment node.
template <typename SorbetAssignmentNode>
unique_ptr<parser::Node> Translator::translateAndOrAssignment(core::LocOffsets location, unique_ptr<parser::Node> lhs,
                                                              unique_ptr<parser::Node> rhs) {
    const auto isOrAsgn = is_same_v<SorbetAssignmentNode, parser::OrAsgn>;
    const auto isAndAsgn = is_same_v<SorbetAssignmentNode, parser::AndAsgn>;
    static_assert(isOrAsgn || isAndAsgn);

    if (!directlyDesugar || !hasExpr(lhs, rhs)) {
        return make_unique<SorbetAssignmentNode>(location, move(lhs), move(rhs));
    }

    auto lhsExpr = lhs->takeDesugaredExpr();
    auto rhsExpr = rhs->takeDesugaredExpr();

    if (preserveConcreteSyntax) {
        auto magicName = isAndAsgn ? core::Names::andAsgn() : core::Names::orAsgn();
        auto locZeroLen = location.copyWithZeroLength();

        // Desugar `x &&= y` to `<Magic>.&&=(x, y)` (likewise for `||=`)
        auto magicSend =
            MK::Send2(location, MK::Magic(locZeroLen), magicName, locZeroLen, move(lhsExpr), move(rhsExpr));
        return make_node_with_expr<SorbetAssignmentNode>(move(magicSend), location, move(lhs), move(rhs));
    }

    if (auto s = ast::cast_tree<ast::Send>(lhsExpr)) {
        auto sendLoc = s->loc;
        auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(rhsExpr));
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
        auto wrapped = MK::InsSeq(location, move(stats), move(if_));
        return make_node_with_expr<SorbetAssignmentNode>(move(wrapped), location, move(lhs), move(rhs));
    }

    if (isa_reference(lhsExpr)) {
        auto lhsCopy = MK::cpRef(lhsExpr);
        auto cond = MK::cpRef(lhsExpr);

        // Check for T.let handling for instance and class variables in ||= assignments
        auto lhsIsIvar = parser::NodeWithExpr::isa_node<parser::IVarLhs>(lhs.get());
        auto lhsIsCvar = parser::NodeWithExpr::isa_node<parser::CVarLhs>(lhs.get());
        auto rhsIsTLet = asTLet(rhsExpr);

        ExpressionPtr assignExpr;
        if (isOrAsgn && (lhsIsIvar || lhsIsCvar) && rhsIsTLet) {
            // Special handling for ||= with T.let on instance/class variables
            // Save the original value before replacing it
            auto originalValue = rhsIsTLet->getPosArg(0).deepCopy();

            // Replace the first argument of T.let with the LHS variable
            rhsIsTLet->getPosArg(0) = MK::cpRef(lhsExpr);

            // Generate pattern: { @z = T.let(@z, ...); <temp> = <original_value>; @z = <temp> }
            auto decl = MK::Assign(location, MK::cpRef(lhsExpr), move(rhsExpr));

            // Create a temporary variable and assign the original value to it
            core::NameRef tempName = nextUniqueDesugarName(core::Names::statTemp());
            auto tempAssign = MK::Assign(location, tempName, move(originalValue));

            // Final assignment from temp to LHS
            auto finalAssign = MK::Assign(location, MK::cpRef(lhsExpr), MK::Local(location, tempName));

            ast::InsSeq::STATS_store stats;
            stats.emplace_back(move(decl));
            stats.emplace_back(move(tempAssign));

            assignExpr = MK::InsSeq(location, move(stats), move(finalAssign));
        } else {
            assignExpr = MK::Assign(location, MK::cpRef(lhsExpr), move(rhsExpr));
        }

        ExpressionPtr if_;
        if constexpr (isAndAsgn) {
            // AndAsgn: if (lhs) { lhs = rhs } else { lhs }
            if_ = MK::If(location, move(cond), move(assignExpr), move(lhsCopy));
        } else {
            // OrAsgn: if (lhs) { lhs } else { lhs = rhs }
            if_ = MK::If(location, move(cond), move(lhsCopy), move(assignExpr));
        }

        return make_node_with_expr<SorbetAssignmentNode>(move(if_), location, move(lhs), move(rhs));
    }

    if (ast::isa_tree<ast::UnresolvedConstantLit>(lhsExpr)) {
        if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
            e.setHeader("Constant reassignment is not supported");
        }
        ExpressionPtr res = MK::EmptyTree();
        return make_node_with_expr<SorbetAssignmentNode>(move(res), location, move(lhs), move(rhs));
    }

    if (ast::isa_tree<ast::InsSeq>(lhsExpr)) {
        auto i = ast::cast_tree<ast::InsSeq>(lhsExpr);
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
        assgnArgs.emplace_back(move(rhsExpr));
        auto cond =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);
        auto tempResult = nextUniqueDesugarName(s->fun);
        stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));
        auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                             numPosAssgnArgs, move(assgnArgs), s->flags);
        auto elsep = MK::Local(sendLoc, tempResult);
        auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
        auto wrapped = MK::InsSeq(location, move(stats), move(iff));
        return make_node_with_expr<SorbetAssignmentNode>(move(wrapped), location, move(lhs), move(rhs));
    }

    Exception::raise("the LHS has been desugared to something we haven't expected: {}", lhsExpr.toString(ctx));
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
unique_ptr<parser::Node> Translator::translateOpAssignment(PrismAssignmentNode *node, core::LocOffsets location,
                                                           unique_ptr<parser::Node> lhs, unique_ptr<parser::Node> rhs) {
    // `OpAsgn` assign needs more information about the specific operator here, so it gets special handling here.
    auto opLoc = translateLoc(node->binary_operator_loc);
    auto op = translateConstantName(node->binary_operator);
    if (!directlyDesugar || !hasExpr(lhs, rhs)) {
        return make_unique<parser::OpAsgn>(location, move(lhs), op, opLoc, move(rhs));
    }

    auto lhsExpr = lhs->takeDesugaredExpr();
    auto rhsExpr = rhs->takeDesugaredExpr();

    if (preserveConcreteSyntax) {
        auto magicName = core::Names::opAsgn();
        auto locZeroLen = location.copyWithZeroLength();
        auto magicSend =
            MK::Send2(location, MK::Magic(locZeroLen), magicName, locZeroLen, move(lhsExpr), move(rhsExpr));
        return make_node_with_expr<parser::OpAsgn>(move(magicSend), location, move(lhs), op, opLoc, move(rhs));
    }

    if (ast::isa_tree<ast::Send>(lhsExpr)) {
        auto s = ast::cast_tree<ast::Send>(lhsExpr);
        auto sendLoc = s->loc;
        auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(s);

        // Create the read operation: obj.method() or obj[index]
        auto prevValue =
            MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs, move(readArgs), s->flags);

        // Apply the operation: prevValue op rhsExpr
        auto newValue = MK::Send1(sendLoc, move(prevValue), op, opLoc, move(rhsExpr));

        // Add the new value to the assignment arguments
        assgnArgs.emplace_back(move(newValue));
        auto numPosAssgnArgs = numPosArgs + 1;

        // Create the assignment operation: obj.method=(newValue) or obj[]=(index, newValue)
        auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                            numPosAssgnArgs, move(assgnArgs), s->flags);

        auto wrapped = MK::InsSeq(location, move(stats), move(res));
        return make_node_with_expr<SorbetAssignmentNode>(move(wrapped), location, move(lhs), op, opLoc, move(rhs));
    }

    if (isa_reference(lhsExpr)) {
        auto lhsCopy = MK::cpRef(lhsExpr);
        auto callOp = MK::Send1(location, move(lhsExpr), op, opLoc, move(rhsExpr));
        auto assign = MK::Assign(location, move(lhsCopy), move(callOp));
        return make_node_with_expr<SorbetAssignmentNode>(move(assign), location, move(lhs), op, opLoc, move(rhs));
    }

    if (ast::isa_tree<ast::UnresolvedConstantLit>(lhsExpr)) {
        if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
            e.setHeader("Constant reassignment is not supported");
        }
        ExpressionPtr res = MK::EmptyTree();
        return make_node_with_expr<SorbetAssignmentNode>(move(res), location, move(lhs), op, opLoc, move(rhs));
    }

    if (auto i = ast::cast_tree<ast::InsSeq>(lhsExpr)) {
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
        auto newValue = MK::Send1(sendLoc, move(prevValue), op, opLoc, move(rhsExpr));
        auto numPosAssgnArgs = numPosArgs + 1;
        assgnArgs.emplace_back(move(newValue));

        auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(ctx), sendLoc.copyWithZeroLength(),
                            numPosAssgnArgs, move(assgnArgs), s->flags);
        auto wrapped = MK::InsSeq(location, move(stats), move(res));
        ifExpr->elsep = move(wrapped);
        return make_node_with_expr<SorbetAssignmentNode>(move(lhsExpr), location, move(lhs), op, opLoc, move(rhs));
    }

    auto s = fmt::format("the LHS has been desugared to something we haven't expected: {}", lhsExpr.toString(ctx));
    Exception::raise(s);
}

template <typename PrismConstantNode, typename SorbetAssignmentNode>
unique_ptr<parser::Node> Translator::translateConstantAssignment(pm_node_t *node, core::LocOffsets location) {
    if (auto e = ctx.beginIndexerError(location, core::errors::Desugar::NoConstantReassignment)) {
        e.setHeader("Constant reassignment is not supported");
    }

    auto constantNode = down_cast<PrismConstantNode>(node);
    constexpr bool replaceWithDynamicConstAssign = true;
    auto lhs = translateConst<PrismConstantNode, parser::ConstLhs, replaceWithDynamicConstAssign>(constantNode);
    return translateAnyOpAssignment<PrismConstantNode, SorbetAssignmentNode, parser::ConstLhs>(constantNode, location,
                                                                                               move(lhs));
}

template <typename PrismConstantPathNode, typename SorbetAssignmentNode>
unique_ptr<parser::Node> Translator::translateConstantPathAssignment(pm_node_t *node, core::LocOffsets location) {
    auto constantPathNode = down_cast<PrismConstantPathNode>(node);
    auto target = constantPathNode->target;
    auto lhs = translateConst<pm_constant_path_node, parser::ConstLhs>(target);
    return translateAnyOpAssignment<PrismConstantPathNode, SorbetAssignmentNode, parser::ConstLhs>(constantPathNode,
                                                                                                   location, move(lhs));
}

template <typename PrismVariableNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
unique_ptr<parser::Node> Translator::translateVariableAssignment(pm_node_t *node, core::LocOffsets location) {
    auto variableNode = down_cast<PrismVariableNode>(node);
    auto nameLoc = translateLoc(variableNode->name_loc);
    auto name = translateConstantName(variableNode->name);

    auto expr = ast::make_expression<ast::UnresolvedIdent>(nameLoc, getIdentKind<SorbetLHSNode>(), name);
    auto lhs = make_node_with_expr<SorbetLHSNode>(move(expr), nameLoc, name);
    return translateAnyOpAssignment<PrismVariableNode, SorbetAssignmentNode, SorbetLHSNode>(variableNode, location,
                                                                                            move(lhs));
}

// Handle operator assignment to the result of a safe method call, like `a&.b += 1`
// This creates a pattern like: { $temp = a; if $temp == nil then nil else $temp.b += 1 }
template <typename PrismAssignmentNode, typename SorbetAssignmentNode>
unique_ptr<parser::Node> Translator::translateCSendAssignment(PrismAssignmentNode *callNode, core::LocOffsets location,
                                                              unique_ptr<parser::Node> receiver, core::NameRef name,
                                                              core::LocOffsets messageLoc) {
    if (!directlyDesugar || !hasExpr(receiver)) {
        // Fall back to CSend if we can't desugar directly
        auto lhs = make_unique<parser::CSend>(location, move(receiver), name, messageLoc, NodeVec{});
        return translateAnyOpAssignment<PrismAssignmentNode, SorbetAssignmentNode, parser::CSend>(callNode, location,
                                                                                                  move(lhs));
    }

    // Create temporary variable to hold the receiver
    auto tempRecv = nextUniqueDesugarName(core::Names::assignTemp());
    auto receiverExpr = receiver->takeDesugaredExpr();
    auto recvLoc = receiver->loc;
    auto zeroLengthLoc = location.copyWithZeroLength();
    auto zeroLengthRecvLoc = recvLoc.copyWithZeroLength();

    // The `&` in `a&.b = 1`
    constexpr auto len = "&"sv.size();
    auto ampersandLoc = translateLoc(callNode->call_operator_loc.start, callNode->call_operator_loc.start + len);

    auto tempAssign = MK::Assign(zeroLengthRecvLoc, tempRecv, move(receiverExpr));
    auto cond = MK::Send1(zeroLengthLoc, MK::Constant(zeroLengthRecvLoc, core::Symbols::NilClass()),
                          core::Names::tripleEq(), zeroLengthRecvLoc, MK::Local(zeroLengthRecvLoc, tempRecv));
    auto send =
        MK::Send(location, MK::Local(zeroLengthRecvLoc, tempRecv), name, messageLoc, 0, ast::Send::ARGS_store{});
    auto tempSend = make_node_with_expr<parser::Send>(move(send), location, nullptr, name, messageLoc, NodeVec{});

    // Recursively handle the assignment operation on the temporary send
    auto assignmentResult = translateAnyOpAssignment<PrismAssignmentNode, SorbetAssignmentNode, parser::Send>(
        callNode, location, move(tempSend));

    auto assignmentExpr = assignmentResult->takeDesugaredExpr();
    auto nilValue = MK::Send1(recvLoc.copyEndWithZeroLength(), MK::Magic(zeroLengthLoc),
                              core::Names::nilForSafeNavigation(), zeroLengthLoc, MK::Local(ampersandLoc, tempRecv));
    auto ifExpr = MK::If(zeroLengthLoc, move(cond), move(nilValue), move(assignmentExpr));
    auto result = MK::InsSeq1(location, move(tempAssign), move(ifExpr));

    // Create a node that directly contains the InsSeq expression for the safe navigation pattern
    auto lhs = make_node_with_expr<parser::Send>(move(result), location, nullptr, name, messageLoc, NodeVec{});
    return move(lhs);
}

// Used the 3 kinds of assignment that lower to `Send` nodes:
// 1. `recv.a &&= b`
// 2. `recv.a ||= b`
// 3. `recv.a  += b`
template <typename PrismAssignmentNode, typename SorbetAssignmentNode>
unique_ptr<parser::Node> Translator::translateSendAssignment(pm_node_t *node, core::LocOffsets location) {
    auto callNode = down_cast<PrismAssignmentNode>(node);
    auto name = translateConstantName(callNode->read_name);
    auto receiver = translate(callNode->receiver);
    auto messageLoc = translateLoc(callNode->message_loc);

    // The assign's location spans from the start of the receiver to the end of
    // the message, not including the operator or RHS:
    //     recv.a += b
    //     ^^^^^^^^^^^ assign loc
    //     ^^^^^^      lhs send loc
    auto lhsLoc = core::LocOffsets{location.beginPos(), messageLoc.endPos()};

    if (PM_NODE_FLAG_P(node, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
        // Handle operator assignment to the result of a safe method call, like `a&.b += 1`
        auto result = translateCSendAssignment<PrismAssignmentNode, SorbetAssignmentNode>(
            callNode, location, move(receiver), name, messageLoc);
        return result;
    }

    // Handle operator assignment to the result of a method call, like `a.b += 1`
    if (!directlyDesugar || !hasExpr(receiver)) {
        auto lhs = make_unique<parser::Send>(lhsLoc, move(receiver), name, messageLoc, NodeVec{});
        auto result = translateAnyOpAssignment<PrismAssignmentNode, SorbetAssignmentNode, parser::Send>(
            callNode, location, move(lhs));
        return result;
    }
    auto receiverExpr = receiver->takeDesugaredExpr();

    ast::Send::Flags flags;
    flags.isPrivateOk = PM_NODE_FLAG_P(node, PM_CALL_NODE_FLAGS_IGNORE_VISIBILITY);

    auto send = MK::Send(lhsLoc, move(receiverExpr), name, messageLoc, 0, ast::Send::ARGS_store{}, flags);
    auto lhs = make_node_with_expr<parser::Send>(move(send), lhsLoc, move(receiver), name, messageLoc, NodeVec{});

    return translateAnyOpAssignment<PrismAssignmentNode, SorbetAssignmentNode, parser::Send>(callNode, location,
                                                                                             move(lhs));
}

unique_ptr<parser::Node> Translator::translate(pm_node_t *node, bool preserveConcreteSyntax) {
    if (node == nullptr)
        return nullptr;

    auto location = translateLoc(node->location);

    switch (PM_NODE_TYPE(node)) {
        case PM_ALIAS_GLOBAL_VARIABLE_NODE: { // The `alias` keyword used for global vars, like `alias $new $old`
            auto aliasGlobalVariableNode = down_cast<pm_alias_global_variable_node>(node);

            auto newName = translate(aliasGlobalVariableNode->new_name);
            auto oldName = translate(aliasGlobalVariableNode->old_name);

            if (!directlyDesugar) {
                return make_unique<parser::Alias>(location, move(newName), move(oldName));
            }

            auto toExpr = newName->takeDesugaredExpr();
            auto fromExpr = oldName->takeDesugaredExpr();

            // Desugar `alias $new $old` to `self.alias_method($new, $old)`
            auto expr = MK::Send2(location, MK::Self(location), core::Names::aliasMethod(),
                                  location.copyWithZeroLength(), std::move(toExpr), std::move(fromExpr));

            return make_node_with_expr<parser::Alias>(move(expr), location, move(newName), move(oldName));
        }
        case PM_ALIAS_METHOD_NODE: { // The `alias` keyword, like `alias new_method old_method`
            auto aliasMethodNode = down_cast<pm_alias_method_node>(node);

            auto newName = translate(aliasMethodNode->new_name);
            auto oldName = translate(aliasMethodNode->old_name);

            if (!directlyDesugar || !hasExpr(newName, oldName)) {
                return make_unique<parser::Alias>(location, move(newName), move(oldName));
            }

            auto toExpr = newName->takeDesugaredExpr();
            auto fromExpr = oldName->takeDesugaredExpr();

            // Desugar methods: `alias new old` to `self.alias_method(new, old)`
            auto expr = MK::Send2(location, MK::Self(location), core::Names::aliasMethod(),
                                  location.copyWithZeroLength(), std::move(toExpr), std::move(fromExpr));

            return make_node_with_expr<parser::Alias>(move(expr), location, move(newName), move(oldName));
        }
        case PM_AND_NODE: { // operator `&&` and `and`
            auto andNode = down_cast<pm_and_node>(node);

            auto left = translate(andNode->left);
            auto right = translate(andNode->right);

            if (!directlyDesugar || !hasExpr(left, right)) {
                return make_unique<parser::And>(location, move(left), move(right));
            }

            auto lhsExpr = left->takeDesugaredExpr();
            auto rhsExpr = right->takeDesugaredExpr();

            if (preserveConcreteSyntax) {
                auto andAndLoc = core::LocOffsets{left->loc.endPos(), right->loc.beginPos()};
                auto magicSend = MK::Send2(location, MK::Magic(location.copyWithZeroLength()), core::Names::andAnd(),
                                           andAndLoc, move(lhsExpr), move(rhsExpr));
                return make_node_with_expr<parser::And>(move(magicSend), location, move(left), move(right));
            }

            if (isa_reference(lhsExpr)) {
                auto cond = MK::cpRef(lhsExpr);
                auto if_ = MK::If(location, move(cond), move(rhsExpr), move(lhsExpr));
                return make_node_with_expr<parser::And>(move(if_), location, move(left), move(right));
            }

            // For non-reference expressions, create a temporary variable so we don't evaluate the LHS twice.
            // E.g. `x = 1 && 2` becomes `x = (temp = 1; temp ? temp : 2)`
            core::NameRef tempLocalName = nextUniqueDesugarName(core::Names::andAnd());

            bool checkAndAnd = ast::isa_tree<ast::Send>(lhsExpr) && ast::isa_tree<ast::Send>(rhsExpr);
            ExpressionPtr thenp;
            if (checkAndAnd) {
                auto lhsSend = ast::cast_tree<ast::Send>(lhsExpr);
                auto rhsSend = ast::cast_tree<ast::Send>(rhsExpr);
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
                    thenp = move(rhsExpr);
                } else {
                    thenp = move(rhsExpr);
                }
            } else {
                thenp = move(rhsExpr);
            }

            auto lhsLoc = left->loc;
            auto rhsLoc = right->loc;
            auto condLoc =
                lhsLoc.exists() && rhsLoc.exists() ? core::LocOffsets{lhsLoc.endPos(), rhsLoc.beginPos()} : lhsLoc;
            auto temp = MK::Assign(location, tempLocalName, move(lhsExpr));
            auto elsep = MK::Local(lhsLoc, tempLocalName);
            auto cond = MK::Local(condLoc, tempLocalName);
            auto if_ = MK::If(location, move(cond), move(thenp), move(elsep));
            auto wrapped = MK::InsSeq1(location, move(temp), move(if_));
            return make_node_with_expr<parser::And>(move(wrapped), location, move(left), move(right));
        }
        case PM_ARGUMENTS_NODE: { // A list of arguments in one of several places:
            // 1. The arguments to a method call, e.g the `1, 2, 3` in `f(1, 2, 3)`.
            // 2. The value(s) returned from a return statement, e.g. the `1, 2, 3` in `return 1, 2, 3`.
            // 3. The arguments to a `yield` call, e.g. the `1, 2, 3` in `yield 1, 2, 3`.
            unreachable("PM_ARGUMENTS_NODE is handled separately in `Translator::translateArguments()`.");
        }
        case PM_ARRAY_NODE: { // An array literal, e.g. `[1, 2, 3]`
            auto arrayNode = down_cast<pm_array_node>(node);

            auto sorbetElements = translateMulti(arrayNode->elements);

            if (!directlyDesugar || !hasExpr(sorbetElements)) {
                return make_unique<parser::Array>(location, move(sorbetElements));
            }

            ast::Array::ENTRY_store elements;
            elements.reserve(arrayNode->elements.size);

            ENFORCE(sorbetElements.size() == arrayNode->elements.size);
            for (auto &stat : sorbetElements) {
                auto expr = stat->takeDesugaredExpr();
                ENFORCE(expr != nullptr);
                elements.emplace_back(move(expr));
            }

            auto prismElements = absl::MakeSpan(arrayNode->elements.nodes, arrayNode->elements.size);
            auto expr = desugarArray(location, prismElements, move(elements));
            return make_node_with_expr<parser::Array>(move(expr), location, move(sorbetElements));
        }
        case PM_ASSOC_NODE: { // A key-value pair in a Hash literal, e.g. the `a: 1` in `{ a: 1 }
            auto assocNode = down_cast<pm_assoc_node>(node);

            auto key = translate(assocNode->key);
            auto value = translate(assocNode->value);

            return make_unique<parser::Pair>(location, move(key), move(value));
        }
        case PM_ASSOC_SPLAT_NODE: { // A Hash splat, e.g. `**h` in `f(a: 1, **h)` and `{ k: v, **h }`
            unreachable("PM_ASSOC_SPLAT_NODE is handled separately in `Translator::translateKeyValuePairs()` and "
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
            auto expr = MK::Send1(location, move(recv), core::Names::regexBackref(), locZeroLen, move(arg));

            return make_node_with_expr<parser::Backref>(move(expr), location, name);
        }
        case PM_BEGIN_NODE: { // A `begin ... end` block
            auto beginNode = down_cast<pm_begin_node>(node);

            NodeVec statements = translateEnsure(beginNode);

            if (statements.empty()) {
                return make_node_with_expr<parser::Kwbegin>(MK::Nil(location), location, std::move(statements));
            }

            if (!directlyDesugar || !hasExpr(statements)) {
                return make_unique<parser::Kwbegin>(location, move(statements));
            }

            auto args = nodeVecToStore<ast::InsSeq::STATS_store>(statements);
            auto finalExpr = std::move(args.back());
            args.pop_back();
            auto expr = MK::InsSeq(location, std::move(args), std::move(finalExpr));
            return make_node_with_expr<parser::Kwbegin>(std::move(expr), location, std::move(statements));
        }
        case PM_BLOCK_ARGUMENT_NODE: { // A block arg passed into a method call, e.g. the `&b` in `a.map(&b)`
            auto blockArg = down_cast<pm_block_argument_node>(node);

            auto expr = translate(blockArg->expression);

            return make_unique<parser::BlockPass>(location, move(expr));
        }
        case PM_BLOCK_NODE: { // An explicit block passed to a method call, i.e. `{ ... }` or `do ... end`
            unreachable("PM_BLOCK_NODE has special handling in translateCallWithBlock, see its docs for details.");
        }
        case PM_BLOCK_LOCAL_VARIABLE_NODE: { // A named block local variable, like `baz` in `|bar; baz|`
            auto blockLocalNode = down_cast<pm_block_local_variable_node>(node);
            auto sorbetName = translateConstantName(blockLocalNode->name);
            return make_node_with_expr<parser::Shadowarg>(MK::ShadowArg(location, MK::Local(location, sorbetName)),
                                                          location, sorbetName);
        }
        case PM_BLOCK_PARAMETER_NODE: { // A block parameter declared at the top of a method, e.g. `def m(&block)`
            unreachable("PM_BLOCK_PARAMETER_NODE is handled separately in `Translator::translateParametersNode()`.");
        }
        case PM_BLOCK_PARAMETERS_NODE: { // The parameters declared at the top of a PM_BLOCK_NODE
            // Like the `|x|` in `foo { |x| ... } `
            auto paramsNode = down_cast<pm_block_parameters_node>(node);

            if (paramsNode->parameters == nullptr) {
                // TODO: future follow up, ensure we add the block local variables ("shadowargs"), if any.
                return make_unique<parser::Params>(location, NodeVec{});
            }

            unique_ptr<parser::Params> params;
            std::tie(params, std::ignore) = translateParametersNode(paramsNode->parameters, location);

            // Sorbet's legacy parser inserts locals ("Shadowargs") at the end of the block's Params node,
            // after all other parameters.
            auto sorbetShadowParams = translateMulti(paramsNode->locals);
            params->params.insert(params->params.end(), make_move_iterator(sorbetShadowParams.begin()),
                                  make_move_iterator(sorbetShadowParams.end()));

            return params;
        }
        case PM_BREAK_NODE: { // A `break` statement, e.g. `break`, `break 1, 2, 3`
            auto breakNode = down_cast<pm_break_node>(node);

            auto arguments = translateArguments(breakNode->arguments);

            if (arguments.empty()) {
                auto expr = MK::Break(location, MK::EmptyTree());
                return make_node_with_expr<parser::Break>(move(expr), location, move(arguments));
            }

            if (!directlyDesugar || !hasExpr(arguments)) {
                return make_unique<parser::Break>(location, move(arguments));
            }

            ExpressionPtr breakArgs;
            if (arguments.size() == 1) {
                auto &first = arguments[0];
                breakArgs = first == nullptr ? MK::EmptyTree() : first->takeDesugaredExpr();
            } else {
                auto args = nodeVecToStore<ast::Array::ENTRY_store>(arguments);
                auto arrayLocation = parser.translateLocation(breakNode->arguments->base.location);
                breakArgs = MK::Array(arrayLocation, std::move(args));
            }
            auto expr = MK::Break(location, std::move(breakArgs));
            return make_node_with_expr<parser::Break>(move(expr), location, move(arguments));
        }
        case PM_CALL_AND_WRITE_NODE: { // And-assignment to a method call, e.g. `a.b &&= false`
            return translateSendAssignment<pm_call_and_write_node, parser::AndAsgn>(node, location);
        }
        case PM_CALL_NODE: { // A method call like `a.b()` or `a&.b()`
            auto callNode = down_cast<pm_call_node>(node);

            auto constantNameString = parser.resolveConstant(callNode->name);
            auto receiver = translate(callNode->receiver);

            core::LocOffsets messageLoc;

            // When the message is empty, like `foo.()`, the message location is the
            // same as the call operator location
            if (callNode->message_loc.start == nullptr && callNode->message_loc.end == nullptr) {
                messageLoc = translateLoc(callNode->call_operator_loc);
            } else {
                messageLoc = translateLoc(callNode->message_loc);
            }

            absl::Span<pm_node_t *> prismArgs;
            if (auto *prismArgsNode = callNode->arguments) {
                prismArgs = absl::MakeSpan(prismArgsNode->arguments.nodes, prismArgsNode->arguments.size);
            }

            // The legacy parser nodes don't include the literal block argument (if any), but the desugar nodes do
            // include it.
            core::LocOffsets sendLoc;  // The location of the "send" node, exluding any literal block, if any.
            core::LocOffsets blockLoc; // The location of just the block node, on its own.
            core::LocOffsets sendWithBlockLoc = location;
            location = core::LocOffsets::none(); // Invalidate this to ensure we don't use it again in this path.
            if (callNode->block == nullptr) { // There's no block, so the `sendLoc` and `sendWithBlockLoc` are the same.
                sendLoc = sendWithBlockLoc;
            } else { // There's a block, so we need to calculate the location of the "send" node, excluding it.
                sendLoc = messageLoc;
                blockLoc = translateLoc(callNode->block->location);

                if (callNode->receiver) {
                    sendLoc = translateLoc(callNode->receiver->location).join(sendLoc);
                }

                if (callNode->closing_loc.start &&
                    callNode->closing_loc.end) { // explicit `( )` or `[ ]` around the params
                    sendLoc = sendLoc.join(translateLoc(callNode->closing_loc));
                }

                if (!prismArgs.empty()) { // Extend to last argument's location, if any.
                    // For index expressions, the closing_loc can come before the last
                    // argument's location:
                    //     a[1, 2] = 3
                    //           ^     closing loc
                    //               ^ last arg loc
                    sendLoc = sendLoc.join(translateLoc(prismArgs.back()->location));
                }

                // The block pass arugment is not stored with the other arguments, so we handle it separately here.
                if (PM_NODE_TYPE_P(callNode->block, PM_BLOCK_ARGUMENT_NODE)) {
                    auto blockPassArgLoc = translateLoc(callNode->block->location);
                    sendLoc = sendLoc.join(blockPassArgLoc);
                }
            }
            auto sendLoc0 = sendLoc.copyWithZeroLength();

            // Handle `~[Integer]`, like `~42`
            // Unlike `-[Integer]`, Prism treats `~[Integer]` as a method call
            // But Sorbet's legacy parser treats both `~[Integer]` and `-[Integer]` as integer literals
            if (constantNameString == "~" && PM_NODE_TYPE_P(callNode->receiver, PM_INTEGER_NODE)) {
                string valueString(sliceLocation(callNode->base.location));

                if (!directlyDesugar) {
                    return make_unique<parser::Integer>(sendLoc, move(valueString));
                }

                // The purely integer part of it, not including the `~`
                auto integerExpr = receiver->takeDesugaredExpr();
                ENFORCE(integerExpr != nullptr, "All Integer nodes should have been desugared already");

                // Model this as an Integer in the parse tree, but desugar to a method call like `42.~()`
                auto sendNode =
                    MK::Send0(sendLoc, move(integerExpr), core::Names::tilde(), sendLoc.copyEndWithZeroLength());
                return make_node_with_expr<parser::Integer>(move(sendNode), sendLoc, move(valueString));
            }

            if (constantNameString == "[]" || constantNameString == "[]=") {
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

            NodeVec args;
            // true if the call contains a forwarded argument like `foo(...)`
            auto hasFwdArgs = callNode->arguments != nullptr &&
                              PM_NODE_FLAG_P(callNode->arguments, PM_ARGUMENTS_NODE_FLAGS_CONTAINS_FORWARDING);
            auto hasFwdRestArg = false; // true if the call contains an anonymous forwarded rest arg like `foo(*)`
            auto hasSplat = false;      // true if the call contains a splatted expression like `foo(*a)`
            unique_ptr<parser::Hash> kwargsHash;
            auto kwargsHashHasExpr = true; // true if we can directly desugar the kwargs Hash, if any.
            if (!prismArgs.empty()) {
                // Pop the Kwargs Hash off the end of the arguments, if there is one.
                if (PM_NODE_TYPE_P(prismArgs.back(), PM_KEYWORD_HASH_NODE)) {
                    auto h = translate(prismArgs.back());
                    auto hash = unique_ptr<parser::Hash>(reinterpret_cast<parser::Hash *>(h.release()));
                    ENFORCE(hash != nullptr);

                    if (hash->kwargs) {
                        kwargsHash = move(hash);

                        // Remove the kwargsHash from the arguments Span, so it's not revisited by the `for` loop below.
                        prismArgs.remove_suffix(1);

                        kwargsHashHasExpr = absl::c_all_of(kwargsHash->pairs, [](auto &node) {
                            // Only support kwarg Hashes that only contain pairs for now.
                            // TODO: Add support for Kwsplat and ForwardedKwrestArg

                            auto pair = parser::NodeWithExpr::cast_node<parser::Pair>(node.get());
                            return pair != nullptr && hasExpr(pair->key, pair->value);
                        });
                    }
                }

                // TODO: reserve size for kwargs Hash keys and values, if needed.
                // TODO: reserve size for block, if needed.
                args.reserve(prismArgs.size());

                for (auto &arg : prismArgs) {
                    switch (PM_NODE_TYPE(arg)) {
                        case PM_SPLAT_NODE: {
                            auto splatNode = down_cast<pm_splat_node>(arg);
                            if (splatNode->expression == nullptr) { // An anonymous splat like `f(*)`
                                hasFwdRestArg = true;
                            } else { // Splatting an expression like `f(*a)`
                                hasSplat = true;
                            }

                            args.emplace_back(translate(arg));

                            break;
                        }

                        default:
                            args.emplace_back(translate(arg));

                            break;
                    }
                }
            }

            pm_node_t *prismBlock = callNode->block;

            unique_ptr<parser::Node> sendNode;

            auto name = ctx.state.enterNameUTF8(constantNameString);
            auto methodName = MK::Symbol(sendLoc0, name);

            // Method defs are really complex, and we're building support for different kinds of arguments bit
            // by bit. This bool is true when this particular method call is supported by our desugar logic.
            auto supportedArgs = absl::c_all_of(args, [](const auto &arg) {
                return hasExpr(arg) || parser::NodeWithExpr::isa_node<parser::ForwardedRestArg>(arg.get()) ||
                       parser::NodeWithExpr::isa_node<parser::ForwardedArgs>(arg.get());
            });
            auto supportedCallType =
                constantNameString != "block_given?" && kwargsHashHasExpr && hasExpr(receiver) && supportedArgs;

            unique_ptr<parser::Node> blockBody;       // e.g. `123` in `foo { |x| 123 }`
            unique_ptr<parser::Node> blockParameters; // e.g. `|x|` in `foo { |x| 123 }`
            ast::MethodDef::PARAMS_store blockParamsStore;
            ast::InsSeq::STATS_store blockStatsStore;
            unique_ptr<parser::Node> blockPassNode;
            ast::ExpressionPtr blockPassArg;
            core::LocOffsets blockPassLoc;
            bool blockPassArgIsSymbol = false;
            bool supportedBlock = false;
            if (prismBlock != nullptr) {
                if (PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE)) { // a literal block with `{ ... }` or `do ... end`

                    auto blockNode = down_cast<pm_block_node>(prismBlock);

                    blockBody = this->enterBlockContext().translate(blockNode->body);

                    supportedCallType &= hasExpr(blockBody);

                    auto attemptToDesugarBlockParams = supportedCallType;
                    bool didDesugarBlockParams = false;

                    if (blockNode->parameters != nullptr) {
                        switch (PM_NODE_TYPE(blockNode->parameters)) {
                            case PM_BLOCK_PARAMETERS_NODE: { // The params declared at the top of a PM_BLOCK_NODE
                                // Like the `|x|` in `foo { |x| ... }`
                                auto paramsNode = down_cast<pm_block_parameters_node>(blockNode->parameters);

                                auto paramsLoc = translateLoc(paramsNode->base.location);

                                if (paramsNode->parameters == nullptr) {
                                    // This can happen if the block declares block-local variables, but no parameters.
                                    // e.g. `foo { |; block_local_var| ... }`

                                    // TODO: future follow up, ensure we add the block local variables ("shadowargs"),
                                    // if any.
                                    blockParameters = make_unique<parser::Params>(paramsLoc, NodeVec{});
                                    didDesugarBlockParams = true;
                                } else {
                                    unique_ptr<parser::Params> params;
                                    std::tie(params, std::ignore) =
                                        translateParametersNode(paramsNode->parameters, paramsLoc);

                                    // Sorbet's legacy parser inserts locals ("Shadowargs") at the end of the block's
                                    // Params node, after all other parameters.
                                    auto sorbetShadowParams = translateMulti(paramsNode->locals);
                                    params->params.insert(params->params.end(),
                                                          make_move_iterator(sorbetShadowParams.begin()),
                                                          make_move_iterator(sorbetShadowParams.end()));

                                    std::tie(blockParamsStore, blockStatsStore, didDesugarBlockParams) =
                                        desugarParametersNode(params->params, attemptToDesugarBlockParams);

                                    blockParameters = move(params);
                                }

                                break;
                            }

                            case PM_NUMBERED_PARAMETERS_NODE: { // The params in a PM_BLOCK_NODE with numbered params
                                // Like the implicit `|_1, _2, _3|` in `foo { _3 }`
                                auto numberedParamsNode = down_cast<pm_numbered_parameters_node>(blockNode->parameters);

                                // Use a 0-length loc just after the `do` or `{` token, as if you had written:
                                //     do|_1, _2| ... end`
                                //       ^
                                //     {|_1, _2| ... }`
                                //      ^
                                auto numParamsLoc =
                                    translateLoc(blockNode->opening_loc.end, blockNode->opening_loc.end);

                                auto params = translateNumberedParametersNode(
                                    numberedParamsNode, down_cast<pm_statements_node>(blockNode->body),
                                    &blockParamsStore);

                                didDesugarBlockParams = true;

                                blockParameters = make_unique<parser::NumParams>(numParamsLoc, move(params));

                                break;
                            }

                            case PM_IT_PARAMETERS_NODE: {
                                unreachable("PM_IT_PARAMETERS_NODE is not implemented yet");
                            }

                            default: {
                                unreachable("Found a {} block parameter type, which is not implemented yet ",
                                            pm_node_type_to_str(PM_NODE_TYPE(blockNode->parameters)));
                            }
                        }

                        supportedBlock = didDesugarBlockParams;

                    } else {
                        supportedBlock = true;
                    }
                } else {
                    ENFORCE(PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)); // the `&b` in `a.map(&b)`

                    auto *bp = down_cast<pm_block_argument_node>(prismBlock);

                    blockPassNode = translate(prismBlock);
                    blockPassLoc = blockPassNode->loc;

                    if (bp->expression) {
                        blockPassArgIsSymbol = PM_NODE_TYPE_P(bp->expression, PM_SYMBOL_NODE);

                        if (blockPassArgIsSymbol) {
                            supportedBlock = true;
                        } else {
                            auto blockPassArgNode = translate(bp->expression);

                            if (supportedCallType && hasExpr(blockPassArgNode)) {
                                blockPassArg = blockPassArgNode->takeDesugaredExpr();
                                supportedBlock = true;
                            } else {
                                supportedBlock = false;
                            }
                        }
                    } else {
                        // Replace an anonymous block pass like `f(&)` with a local variable
                        // reference, like `f(&<&>)`.
                        blockPassArg = MK::Local(blockPassLoc.copyEndWithZeroLength(), core::Names::ampersand());
                        supportedBlock = true;
                    }
                }
            } else {
                // There is no block, so we support direct desugaring of this method call.
                supportedBlock = true;
            }

            if (hasFwdArgs) { // Desugar a call like `foo(...)` so it has a block argument like `foo(..., &b)`.
                ENFORCE(blockPassArg == nullptr, "The parser should have rejected a call with both a block pass "
                                                 "argument and forwarded args (e.g. `foo(&b, ...)`)");

                blockPassArg = MK::Local(sendWithBlockLoc, core::Names::fwdBlock());
                blockPassLoc = sendLoc.copyEndWithZeroLength();
                supportedBlock = true;
            }

            supportedCallType &= supportedBlock;

            // TODO: Direct desugaring support for conditional sends is not implemented yet.
            supportedCallType &= !PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION);

            if (!directlyDesugar || !supportedCallType) {
                // We previously popped the kwargs Hash off, in the hopes that we can directly desugar it.
                // Turns out we can't, so let's put it back (and in the correct order).
                if (kwargsHash) {
                    args.emplace_back(move(kwargsHash));
                }

                if (prismBlock && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                    // PM_BLOCK_ARGUMENT_NODE models the `&b` in `a.map(&b)`,
                    // but not a literal block with `{ ... }` or `do ... end`

                    args.emplace_back(move(blockPassNode));
                }

                if (PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                    sendNode = make_unique<parser::CSend>(sendLoc, move(receiver), name, messageLoc, move(args));
                } else {
                    sendNode = make_unique<parser::Send>(sendLoc, move(receiver), name, messageLoc, move(args));
                }

                if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE)) {
                    // PM_BLOCK_NODE models an explicit block arg with `{ ... }` or
                    // `do ... end`, but not a forwarded block like the `&b` in `a.map(&b)`.
                    // In Prism, this is modeled by a `pm_call_node` with a `pm_block_node` as a child, but the
                    // The legacy parser inverts this , with a parent "Block" with a child
                    // "Send".
                    return translateCallWithBlock(prismBlock, move(sendNode));
                }

                return sendNode;
            }

            ast::Send::Flags flags;

            ast::ExpressionPtr receiverExpr;
            if (receiver == nullptr) { // Convert `foo()` to `self.foo()`
                // 0-sized Loc, since `self.` doesn't appear in the original file.
                receiverExpr = MK::Self(sendLoc0);
            } else {
                receiverExpr = receiver->takeDesugaredExpr();
            }

            // Unsupported nodes are desugared to an empty tree.
            // Treat them as if they were `self` to match `Desugar.cc`.
            // TODO: Clean up after direct desugaring is complete.
            // https://github.com/Shopify/sorbet/issues/671
            if (ast::isa_tree<ast::EmptyTree>(receiverExpr)) {
                receiverExpr = MK::Self(sendLoc0);
                flags.isPrivateOk = true;
            } else {
                flags.isPrivateOk = PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_IGNORE_VISIBILITY);
            }

            if (hasSplat || hasFwdRestArg || hasFwdArgs) { // f(*a) || f(*) || f(...)
                // If we have a splat anywhere in the argument list, desugar the argument list as a single Array node,
                // and synthesize a call to `::Magic.<callWithSplat>(receiver, method, argArray[, &blk])`
                // The `callWithSplat` implementation (in C++) will unpack a tuple type and call into the normal
                // call mechanism.

                ast::Array::ENTRY_store argExprs;
                argExprs.reserve(prismArgs.size());
                for (auto &arg : args) {
                    if (parser::NodeWithExpr::isa_node<parser::ForwardedRestArg>(arg.get())) {
                        continue; // Skip anonymous splats (like `f(*)`), which are handled separately in `PM_CALL_NODE`
                    } else if (parser::NodeWithExpr::isa_node<parser::ForwardedArgs>(arg.get())) {
                        continue; // Skip forwarded args (like `f(...)`), which are handled separately in `PM_CALL_NODE`
                    }

                    auto expr = arg->takeDesugaredExpr();
                    ENFORCE(expr != nullptr);
                    argExprs.emplace_back(move(expr));
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
                    auto argsConcat =
                        MK::Send1(loc, move(argsArrayExpr), core::Names::concat(), sendLoc0, move(tUnsafe));

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
                    auto argsConcat = argsEmpty ? move(argsSplat)
                                                : MK::Send1(loc, move(argsArrayExpr), core::Names::concat(), sendLoc0,
                                                            move(argsSplat));

                    // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
                    //                                                                                    ^^^^^^^^^^^^
                    auto fwdKwargs = MK::Local(loc, core::Names::fwdKwargs());

                    // `argsArrayExpr.concat(::Magic.<splat>(<fwd-args>)).concat([::<Magic>.<to-hash-dup>(<fwd-kwargs>)])`
                    //                                                            ^^^^^^^^^^^^^^^^^^^^^^^^            ^
                    auto kwargsSplat =
                        MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), sendLoc0, move(fwdKwargs));

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
                if (kwargsHash != nullptr) {
                    ast::Array::ENTRY_store kwargElements;
                    flattenKwargs(kwargsHash, kwargElements);
                    ast::desugar::DuplicateHashKeyCheck::checkSendArgs(ctx, 0, kwargElements);

                    // Add the kwargs Hash back into parse tree, so that it's correct, too.
                    // This doesn't effect the desugared expression.
                    args.emplace_back(move(kwargsHash));

                    kwargsExpr = MK::Array(sendWithBlockLoc, move(kwargElements));
                } else {
                    kwargsExpr = MK::Nil(sendWithBlockLoc);
                }

                if (prismBlock && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                    // Add the parser node back into the wq tree, to pass the parser tests.
                    args.emplace_back(move(blockPassNode));
                }

                auto numPosArgs = 4;
                ast::Send::ARGS_store magicSendArgs;
                magicSendArgs.reserve(numPosArgs); // TODO: reserve room for a block pass arg
                magicSendArgs.emplace_back(move(receiverExpr));
                magicSendArgs.emplace_back(move(methodName));
                magicSendArgs.emplace_back(move(argsArrayExpr));
                magicSendArgs.emplace_back(move(kwargsExpr));

                if ((prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE) &&
                     !blockPassArgIsSymbol) ||
                    hasFwdArgs) {
                    // Special handling for non-Symbol block pass args, like `a.map(&block)`
                    // Symbol procs like `a.map(:to_s)` are rewritten into literal block arguments,
                    // and handled separately below.

                    // Desugar a call with a splat, and any other expression as a block pass argument.
                    // E.g. `foo(*splat, &block)`

                    magicSendArgs.emplace_back(move(blockPassArg));
                    numPosArgs++;

                    auto sendExpr =
                        MK::Send(sendWithBlockLoc, MK::Magic(blockPassLoc), core::Names::callWithSplatAndBlockPass(),
                                 messageLoc, numPosArgs, move(magicSendArgs), flags);
                    return make_node_with_expr<parser::Send>(move(sendExpr), sendWithBlockLoc, move(receiver), name,
                                                             messageLoc, move(args));
                }

                if (prismBlock != nullptr) {
                    if (PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE) || blockPassArgIsSymbol) {
                        // A literal block arg (like `foo { ... }` or `foo do ... end`),
                        // or a Symbol proc like `&:b` (which we'll desugar into a literal block)

                        ast::ExpressionPtr blockExpr;
                        if (blockPassArgIsSymbol) {
                            ENFORCE(PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE));
                            auto *bp = down_cast<pm_block_argument_node>(prismBlock);
                            ENFORCE(bp->expression && PM_NODE_TYPE_P(bp->expression, PM_SYMBOL_NODE));

                            auto symbol = down_cast<pm_symbol_node>(bp->expression);
                            blockExpr = desugarSymbolProc(symbol);
                        } else {
                            auto blockLoc = translateLoc(prismBlock->location);
                            auto blockBodyExpr =
                                blockBody == nullptr ? MK::EmptyTree() : blockBody->takeDesugaredExpr();
                            blockExpr = MK::Block(blockLoc, move(blockBodyExpr), move(blockParamsStore));
                        }

                        magicSendArgs.emplace_back(move(blockExpr));
                        flags.hasBlock = true;
                    } else if (PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                        // A forwarded block like the `&b` in `a.map(&b)`
                        unreachable("This should have already been desugared to `Magic.callWithBlockPass()` above.");
                    } else {
                        unreachable("Found an unexpected block of type {}",
                                    pm_node_type_to_str(PM_NODE_TYPE(prismBlock)));
                    }
                }

                // Desugar any call with a splat and without a block pass argument.
                // If there's a literal block argument, that's handled here, too.
                // E.g. `foo(*splat)` or `foo(*splat) { |x| puts(x) }`
                auto sendExpr = MK::Send(sendWithBlockLoc, MK::Magic(sendWithBlockLoc), core::Names::callWithSplat(),
                                         messageLoc, numPosArgs, move(magicSendArgs), flags);
                auto sendNode = make_node_with_expr<parser::Send>(move(sendExpr), sendWithBlockLoc, move(receiver),
                                                                  name, messageLoc, move(args));

                if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE)) {
                    // In Prism, this is modeled by a `pm_call_node` with a `pm_block_node` as a child,
                    // but the legacy parser inverts this, with a parent "Block" with a child "Send".
                    //
                    // Note: The legacy parser doesn't treat block pass arguments this way.
                    //       It just puts them at the end of the arguments list,
                    //       which is why we checked for `PM_BLOCK_NODE` specifically here.

                    return make_node_with_expr<parser::Block>(sendNode->takeDesugaredExpr(), blockLoc, move(sendNode),
                                                              move(blockParameters), move(blockBody));
                }

                return sendNode;
            }

            // Grab a copy of the argument count, before we concat in the kwargs key/value pairs. // huh?
            int numPosArgs = args.size();

            if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE) && !blockPassArgIsSymbol) {
                // Special handling for non-Symbol block pass args, like `a.map(&block)`
                // Symbol procs like `a.map(:to_s)` are rewritten into literal block arguments,
                // and handled separately below.

                // Desugar a call without a splat, and any other expression as a block pass argument.
                // E.g. `a.each(&block)`

                ast::Send::ARGS_store magicSendArgs;
                magicSendArgs.reserve(3 + args.size());
                magicSendArgs.emplace_back(move(receiverExpr));
                magicSendArgs.emplace_back(move(methodName));
                magicSendArgs.emplace_back(move(blockPassArg));

                numPosArgs += 3;

                for (auto &arg : args) {
                    auto x = arg->takeDesugaredExpr();
                    ENFORCE(x != nullptr);
                    magicSendArgs.emplace_back(move(x));
                }

                if (kwargsHash) {
                    flattenKwargs(kwargsHash, magicSendArgs);
                    ast::desugar::DuplicateHashKeyCheck::checkSendArgs(ctx, numPosArgs, magicSendArgs);

                    // Add the kwargs Hash back into parse tree, so that it's correct, too.
                    // This doesn't effect the desugared expression.
                    args.emplace_back(move(kwargsHash));
                }

                if (prismBlock && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                    // Add the parser node back into the wq tree, to pass the parser tests.

                    args.emplace_back(move(blockPassNode));
                }

                auto sendExpr = MK::Send(sendWithBlockLoc, MK::Magic(blockPassLoc), core::Names::callWithBlockPass(),
                                         messageLoc, numPosArgs, move(magicSendArgs), flags);

                return make_node_with_expr<parser::Send>(move(sendExpr), sendWithBlockLoc, move(receiver), name,
                                                         messageLoc, move(args));
            }

            ast::Send::ARGS_store sendArgs{};
            // TODO: reserve size for kwargs Hash keys and values, if needed.
            // TODO: reserve size for the block, if needed.
            sendArgs.reserve(args.size());
            for (auto &arg : args) {
                sendArgs.emplace_back(arg->takeDesugaredExpr());
            }

            if (kwargsHash) {
                flattenKwargs(kwargsHash, sendArgs);
                ast::desugar::DuplicateHashKeyCheck::checkSendArgs(ctx, numPosArgs, sendArgs);

                // Add the kwargs Hash back into parse tree, so that it's correct, too.
                // This doesn't effect the desugared expression.
                args.emplace_back(move(kwargsHash));
            }

            if (prismBlock != nullptr) {
                if (PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE) || blockPassArgIsSymbol) {
                    // A literal block arg (like `foo { ... }` or `foo do ... end`),
                    // or a Symbol proc like `&:b` (which we'll desugar into a literal block)

                    ast::ExpressionPtr blockExpr;
                    if (blockPassArgIsSymbol) {
                        ENFORCE(PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE));
                        auto *bp = down_cast<pm_block_argument_node>(prismBlock);
                        ENFORCE(bp->expression && PM_NODE_TYPE_P(bp->expression, PM_SYMBOL_NODE));

                        auto symbol = down_cast<pm_symbol_node>(bp->expression);
                        blockExpr = desugarSymbolProc(symbol);
                    } else {
                        auto blockLoc = translateLoc(prismBlock->location);
                        auto blockBodyExpr = blockBody == nullptr ? MK::EmptyTree() : blockBody->takeDesugaredExpr();
                        blockExpr = MK::Block(blockLoc, move(blockBodyExpr), move(blockParamsStore));
                    }

                    sendArgs.emplace_back(move(blockExpr));
                    flags.hasBlock = true;
                } else if (PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                    // A forwarded block like the `&b` in `a.map(&b)`
                    unreachable("This should be desugar to `Magic.callWithBlockPass()` above.");
                } else {
                    unreachable("Found an unexpected block of type {}", pm_node_type_to_str(PM_NODE_TYPE(prismBlock)));
                }
            }

            auto expr =
                MK::Send(sendWithBlockLoc, move(receiverExpr), name, messageLoc, numPosArgs, move(sendArgs), flags);

            sendNode =
                make_node_with_expr<parser::Send>(move(expr), sendLoc, move(receiver), name, messageLoc, move(args));

            if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE)) {
                // In Prism, this is modeled by a `pm_call_node` with a `pm_block_node` as a child,
                // but the legacy parser inverts this, with a parent "Block" with a child "Send".
                //
                // Note: The legacy parser doesn't treat block pass arguments this way.
                //       It just puts them at the end of the arguments list,
                //       which is why we checked for `PM_BLOCK_NODE` specifically here.

                return make_node_with_expr<parser::Block>(sendNode->takeDesugaredExpr(), blockLoc, move(sendNode),
                                                          move(blockParameters), move(blockBody));
            }

            return sendNode;
        }
        case PM_CALL_OPERATOR_WRITE_NODE: { // Compound assignment to a method call, e.g. `a.b += 1`
            return translateSendAssignment<pm_call_operator_write_node, parser::OpAsgn>(node, location);
        }
        case PM_CALL_OR_WRITE_NODE: { // Or-assignment to a method call, e.g. `a.b ||= true`
            return translateSendAssignment<pm_call_or_write_node, parser::OrAsgn>(node, location);
        }
        case PM_CALL_TARGET_NODE: { // Target of an indirect write to the result of a method call
            // ... like `self.target1, self.target2 = 1, 2`, `rescue => self.target`, etc.
            auto callTargetNode = down_cast<pm_call_target_node>(node);

            auto receiver = translate(callTargetNode->receiver);
            auto name = translateConstantName(callTargetNode->name);
            auto messageLoc = translateLoc(callTargetNode->message_loc);

            if (PM_NODE_FLAG_P(callTargetNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                // Handle conditional send, e.g. `self&.target1, self&.target2 = 1, 2`
                // It's not valid Ruby, but the parser needs to support it for the diagnostics to work
                return make_unique<parser::CSend>(location, move(receiver), name, messageLoc, NodeVec{});
            } else { // Regular send, e.g. `self.target1, self.target2 = 1, 2`
                return make_unique<parser::Send>(location, move(receiver), name, messageLoc, NodeVec{});
            }
        }
        case PM_CASE_MATCH_NODE: { // A pattern-matching `case` statement that only uses `in` (and not `when`)
            auto caseMatchNode = down_cast<pm_case_match_node>(node);

            auto predicate = translate(caseMatchNode->predicate);
            auto inNodes = patternTranslateMulti(caseMatchNode->conditions);
            auto elseClause = translate(up_cast(caseMatchNode->else_clause));

            // We do not need to check if all the "in" patterns have desugared expressions because they are currently
            // unused.
            if (!directlyDesugar) {
                return make_unique<parser::CaseMatch>(location, move(predicate), move(inNodes), move(elseClause));
            }

            // Build an if ladder similar to CASE_NODE
            core::NameRef tempName;
            core::LocOffsets predicateLoc;

            if (predicate != nullptr) {
                predicateLoc = predicate->loc;
                tempName = nextUniqueDesugarName(core::Names::assignTemp());
            } else {
                tempName = core::NameRef::noName();
            }

            // Start with the else clause as the final else
            ExpressionPtr resultExpr = elseClause == nullptr ? MK::EmptyTree() : elseClause->takeDesugaredExpr();

            // Build the if ladder backwards from the last "in" to the first
            for (auto it = inNodes.rbegin(); it != inNodes.rend(); ++it) {
                auto inPattern = parser::NodeWithExpr::cast_node<parser::InPattern>(it->get());
                ENFORCE(inPattern != nullptr, "case pattern without an in?");

                // Use RaiseUnimplemented as the condition (like desugarOnelinePattern)
                auto patternLoc = inPattern->pattern != nullptr ? inPattern->pattern->loc : inPattern->loc;
                auto matchExpr = MK::RaiseUnimplemented(patternLoc);

                // The body is the statements from the "in" clause
                auto thenExpr = inPattern->body != nullptr ? inPattern->body->takeDesugaredExpr() : MK::EmptyTree();

                // Collect pattern variable assignments from the pattern
                ast::InsSeq::STATS_store vars;
                collectPatternMatchingVars(vars, inPattern->pattern.get());
                if (!vars.empty()) {
                    thenExpr = MK::InsSeq(patternLoc, move(vars), move(thenExpr));
                }

                resultExpr = MK::If(inPattern->loc, move(matchExpr), move(thenExpr), move(resultExpr));
            }

            // Wrap in an InsSeq with the predicate assignment (if there is a predicate)
            if (predicate != nullptr) {
                auto assignExpr = MK::Assign(predicateLoc, tempName, predicate->takeDesugaredExpr());
                resultExpr = MK::InsSeq1(location, move(assignExpr), move(resultExpr));
            }

            return make_node_with_expr<parser::CaseMatch>(move(resultExpr), location, move(predicate), move(inNodes),
                                                          move(elseClause));
        }
        case PM_CASE_NODE: { // A classic `case` statement that only uses `when` (and not pattern matching with `in`)
            auto caseNode = down_cast<pm_case_node>(node);

            auto predicate = translate(caseNode->predicate);

            auto prismWhenNodes = absl::MakeSpan(caseNode->conditions.nodes, caseNode->conditions.size);

            NodeVec whenNodes;
            whenNodes.reserve(prismWhenNodes.size());

            size_t totalPatterns = 0;
            bool allWhensHaveDesugaredExpr = true;

            for (auto *whenNodePtr : prismWhenNodes) {
                auto *whenNode = down_cast<pm_when_node>(whenNodePtr);
                auto whenLoc = translateLoc(whenNode->base.location);

                auto prismPatterns = absl::MakeSpan(whenNode->conditions.nodes, whenNode->conditions.size);

                NodeVec patternNodes;
                patternNodes.reserve(prismPatterns.size());
                translateMultiInto(patternNodes, prismPatterns);
                totalPatterns += patternNodes.size();

                auto statementsNode = translateStatements(whenNode->statements);
                allWhensHaveDesugaredExpr =
                    allWhensHaveDesugaredExpr && hasExpr(statementsNode) && hasExpr(patternNodes);

                // A single `when` clause does not desugar into a standalone Ruby expression; it only
                // becomes meaningful when the enclosing `case` stitches together all clauses. Wrapping it
                // in a NodeWithExpr seeded with `EmptyTree` satisfies the API contract so that
                // `hasExpr(whenNodes)` can succeed. The enclosing `case` later consumes the real
                // expressions from the patterns and body when it assembles the final AST.
                whenNodes.emplace_back(make_node_with_expr<parser::When>(MK::EmptyTree(), whenLoc, move(patternNodes),
                                                                         move(statementsNode)));
            }

            auto elseClause = translate(up_cast(caseNode->else_clause));

            if (!directlyDesugar || !allWhensHaveDesugaredExpr || !hasExpr(predicate, elseClause)) {
                return make_unique<Case>(location, move(predicate), move(whenNodes), move(elseClause));
            }

            if (preserveConcreteSyntax) {
                auto locZeroLen = location.copyWithZeroLength();

                ast::Send::ARGS_store args;
                args.reserve(2 + whenNodes.size() + totalPatterns); // +2 is for the predicate and the patterns count
                args.emplace_back(predicate == nullptr ? MK::EmptyTree() : predicate->takeDesugaredExpr());
                args.emplace_back(MK::Int(locZeroLen, totalPatterns));

                for (auto &whenNodePtr : whenNodes) {
                    auto whenNodeWrapped = parser::NodeWithExpr::cast_node<parser::When>(whenNodePtr.get());
                    ENFORCE(whenNodeWrapped != nullptr, "case without a when?");
                    // Each pattern node already has a desugared expression (populated by translateMulti +
                    // NodeWithExpr). Consume them now; the wrapper's placeholder expression is intentionally ignored.
                    for (auto &patternNode : whenNodeWrapped->patterns) {
                        args.emplace_back(patternNode == nullptr ? MK::EmptyTree() : patternNode->takeDesugaredExpr());
                    }
                }

                for (auto &whenNodePtr : whenNodes) {
                    auto whenNodeWrapped = parser::NodeWithExpr::cast_node<parser::When>(whenNodePtr.get());
                    ENFORCE(whenNodeWrapped != nullptr, "case without a when?");
                    // The body node also carries a real expression once translateStatements has run.
                    auto bodyExpr =
                        whenNodeWrapped->body == nullptr ? MK::EmptyTree() : whenNodeWrapped->body->takeDesugaredExpr();
                    args.emplace_back(move(bodyExpr));
                }

                args.emplace_back(elseClause == nullptr ? MK::EmptyTree() : elseClause->takeDesugaredExpr());

                // Desugar to `::Magic.caseWhen(predicate, num_patterns, patterns..., bodies..., else)`
                auto expr = MK::Send(location, MK::Magic(locZeroLen), core::Names::caseWhen(), locZeroLen, args.size(),
                                     move(args));

                return make_node_with_expr<Case>(move(expr), location, move(predicate), move(whenNodes),
                                                 move(elseClause));
            }

            core::NameRef tempName;
            core::LocOffsets predicateLoc;
            bool hasPredicate = (predicate != nullptr);

            if (hasPredicate) {
                predicateLoc = predicate->loc;
                tempName = nextUniqueDesugarName(core::Names::assignTemp());
            } else {
                tempName = core::NameRef::noName();
            }

            // The if/else ladder for the entire case statement, starting with the else clause as the final `else` when
            // building backwards
            ExpressionPtr resultExpr = elseClause == nullptr ? MK::EmptyTree() : elseClause->takeDesugaredExpr();

            for (auto it = whenNodes.rbegin(); it != whenNodes.rend(); ++it) {
                auto whenNodeWrapped = parser::NodeWithExpr::cast_node<parser::When>(it->get());
                ENFORCE(whenNodeWrapped != nullptr, "case without a when?");

                ExpressionPtr patternsResult; // the if/else ladder for this when clause's patterns
                for (auto &patternNode : whenNodeWrapped->patterns) {
                    auto patternExpr = patternNode == nullptr ? MK::EmptyTree() : patternNode->takeDesugaredExpr();
                    auto patternLoc = patternExpr.loc();

                    ExpressionPtr testExpr;
                    if (parser::NodeWithExpr::isa_node<parser::Splat>(patternNode.get())) {
                        // splat pattern in when clause, predicate is required, `case a when *others`
                        ENFORCE(hasPredicate, "splats need something to test against");
                        auto local = MK::Local(predicateLoc, tempName);
                        // Desugar `case x when *patterns` to `::Magic.<check-match-array>(x, patterns)`,
                        // which behaves like `patterns.any?(x)`
                        testExpr = MK::Send2(patternLoc, MK::Magic(location), core::Names::checkMatchArray(),
                                             patternLoc.copyWithZeroLength(), move(local), move(patternExpr));
                    } else if (hasPredicate) {
                        // regular pattern when case predicate is present, `case a when 1`
                        auto local = MK::Local(predicateLoc, tempName);
                        // Desugar `case x when 1` to `1 === x`
                        testExpr = MK::Send1(patternLoc, move(patternExpr), core::Names::tripleEq(),
                                             patternLoc.copyWithZeroLength(), move(local));
                    } else {
                        // regular pattern when case predicate is not present, `case when 1 then "one" end`
                        // case # no predicate present
                        // when 1
                        //   "one"
                        // end
                        testExpr = move(patternExpr);
                    }

                    if (patternsResult == nullptr) {
                        patternsResult = move(testExpr);
                    } else {
                        auto trueExpr = MK::True(testExpr.loc());
                        patternsResult = MK::If(testExpr.loc(), move(testExpr), move(trueExpr), move(patternsResult));
                    }
                }

                auto thenExpr =
                    whenNodeWrapped->body != nullptr ? whenNodeWrapped->body->takeDesugaredExpr() : MK::EmptyTree();
                resultExpr = MK::If(whenNodeWrapped->loc, move(patternsResult), move(thenExpr), move(resultExpr));
            }

            if (hasPredicate) {
                auto assignExpr = MK::Assign(predicateLoc, tempName, predicate->takeDesugaredExpr());
                resultExpr = MK::InsSeq1(location, move(assignExpr), move(resultExpr));
            }

            return make_node_with_expr<Case>(move(resultExpr), location, move(predicate), move(whenNodes),
                                             move(elseClause));
        }
        case PM_CLASS_NODE: { // Class declarations, not including singleton class declarations (`class <<`)
            auto classNode = down_cast<pm_class_node>(node);

            auto name = translate(classNode->constant_path);
            auto declLoc = translateLoc(classNode->class_keyword_loc).join(name->loc);
            auto superclass = translate(classNode->superclass);
            auto body = this->enterClassContext().translate(classNode->body);

            if (superclass != nullptr) {
                declLoc = declLoc.join(superclass->loc);
            }

            if (!directlyDesugar || !hasExpr(name, superclass, body)) {
                return make_unique<parser::Class>(location, declLoc, move(name), move(superclass), move(body));
            }

            auto bodyExprsOpt = desugarScopeBodyToRHSStore(classNode->body, body);
            if (!bodyExprsOpt.has_value()) {
                return make_unique<parser::Class>(location, declLoc, move(name), move(superclass), move(body));
            }
            auto bodyExprs = move(*bodyExprsOpt);

            ast::ClassDef::ANCESTORS_store ancestors;
            if (superclass == nullptr) {
                ancestors.emplace_back(MK::Constant(location, core::Symbols::todo()));
            } else {
                ancestors.emplace_back(superclass->takeDesugaredExpr());
            }

            auto nameExpr = name->takeDesugaredExpr();
            auto classDef = MK::Class(location, declLoc, move(nameExpr), move(ancestors), move(bodyExprs));

            return make_node_with_expr<parser::Class>(move(classDef), location, declLoc, move(name), move(superclass),
                                                      move(body));
        }
        case PM_CLASS_VARIABLE_AND_WRITE_NODE: { // And-assignment to a class variable, e.g. `@@a &&= 1`
            return translateVariableAssignment<pm_class_variable_and_write_node, parser::AndAsgn, parser::CVarLhs>(
                node, location);
        }
        case PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a class variable, e.g. `@@a += 1`
            return translateVariableAssignment<pm_class_variable_operator_write_node, parser::OpAsgn, parser::CVarLhs>(
                node, location);
        }
        case PM_CLASS_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a class variable, e.g. `@@a ||= 1`
            return translateVariableAssignment<pm_class_variable_or_write_node, parser::OrAsgn, parser::CVarLhs>(
                node, location);
        }
        case PM_CLASS_VARIABLE_READ_NODE: { // A class variable, like `@@a`
            auto classVarNode = down_cast<pm_class_variable_read_node>(node);
            auto name = translateConstantName(classVarNode->name);
            auto expr = ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Class, name);

            return make_node_with_expr<parser::CVar>(move(expr), location, name);
        }
        case PM_CLASS_VARIABLE_TARGET_NODE: { // Target of an indirect write to a class variable
            // ... like `@@target1, @@target2 = 1, 2`, `rescue => @@target`, etc.
            auto classVariableTargetNode = down_cast<pm_class_variable_target_node>(node);
            auto name = translateConstantName(classVariableTargetNode->name);
            auto expr = ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Class, name);
            return make_node_with_expr<parser::CVarLhs>(move(expr), location, name);
        }
        case PM_CLASS_VARIABLE_WRITE_NODE: { // Regular assignment to a class variable, e.g. `@@a = 1`
            return translateAssignment<pm_class_variable_write_node, parser::CVarLhs>(node);
        }
        case PM_CONSTANT_PATH_AND_WRITE_NODE: { // And-assignment to a constant path, e.g. `A::B &&= false`
            return translateConstantPathAssignment<pm_constant_path_and_write_node, parser::AndAsgn>(node, location);
        }
        case PM_CONSTANT_PATH_NODE: { // Part of a constant path, like the `A::B` in `A::B::C`.
            // See`PM_CONSTANT_READ_NODE`, which handles the `::C` part
            auto constantPathNode = down_cast<pm_constant_path_node>(node);

            return translateConst<pm_constant_path_node, parser::Const>(constantPathNode);
        }
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE: { // Compound assignment to a constant path, e.g. `A::B += 1`
            return translateConstantPathAssignment<pm_constant_path_operator_write_node, parser::OpAsgn>(node,
                                                                                                         location);
        }
        case PM_CONSTANT_PATH_OR_WRITE_NODE: { // Or-assignment to a constant path, e.g. `A::B ||= true`
            return translateConstantPathAssignment<pm_constant_path_or_write_node, parser::OrAsgn>(node, location);
        }
        case PM_CONSTANT_PATH_TARGET_NODE: { // Target of an indirect write to a constant path
            // ... like `A::TARGET1, A::TARGET2 = 1, 2`, `rescue => A::TARGET`, etc.
            auto constantPathTargetNode = down_cast<pm_constant_path_target_node>(node);

            return translateConst<pm_constant_path_target_node, parser::ConstLhs>(constantPathTargetNode);
        }
        case PM_CONSTANT_PATH_WRITE_NODE: { // Regular assignment to a constant path, e.g. `A::B = 1`
            return translateAssignment<pm_constant_path_write_node, void>(node);
        }
        case PM_CONSTANT_TARGET_NODE: { // Target of an indirect write to a constant
            // ... like `TARGET1, TARGET2 = 1, 2`, `rescue => TARGET`, etc.
            auto constantTargetNode = down_cast<pm_constant_target_node>(node);
            return translateConst<pm_constant_target_node, parser::ConstLhs>(constantTargetNode);
        }
        case PM_CONSTANT_AND_WRITE_NODE: { // And-assignment to a constant, e.g. `C &&= false`
            return translateConstantAssignment<pm_constant_and_write_node, parser::AndAsgn>(node, location);
        }
        case PM_CONSTANT_OPERATOR_WRITE_NODE: { // Compound assignment to a constant, e.g. `C += 1`
            return translateConstantAssignment<pm_constant_operator_write_node, parser::OpAsgn>(node, location);
        }
        case PM_CONSTANT_OR_WRITE_NODE: { // Or-assignment to a constant, e.g. `C ||= true`
            return translateConstantAssignment<pm_constant_or_write_node, parser::OrAsgn>(node, location);
        }
        case PM_CONSTANT_READ_NODE: { // A single, unnested, non-fully qualified constant like `Foo`
            auto constantReadNode = down_cast<pm_constant_read_node>(node);
            return translateConst<pm_constant_read_node, parser::Const>(constantReadNode);
        }
        case PM_CONSTANT_WRITE_NODE: { // Regular assignment to a constant, e.g. `Foo = 1`
            return translateAssignment<pm_constant_write_node, parser::ConstLhs>(node);
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

            auto receiver = translate(defNode->receiver); // The singleton receiver, like `self` in `self.foo()`
            auto name = translateConstantName(defNode->name);

            auto isSingletonMethod = receiver != nullptr;

            unique_ptr<parser::Params> params;
            core::NameRef enclosingBlockParamName;
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

                std::tie(params, enclosingBlockParamName) = translateParametersNode(defNode->parameters, loc);
            } else {
                if (rparenLoc.start != nullptr) {
                    // The definition has no parameters but still has parentheses, e.g. `def foo(); end`
                    // In this case, Sorbet's legacy parser will still hold an empty Args node
                    auto loc = translateLoc(defNode->lparen_loc.start, defNode->rparen_loc.end);
                    params = make_unique<parser::Params>(loc, NodeVec{});
                } else {
                    params = nullptr;
                }

                // Desugaring a method def like `def foo()` should behave like `def foo(&<blk>)`,
                // so we set a synthetic name here for `yield` to use.
                enclosingBlockParamName = core::Names::blkArg();

                // TODO: In a future PR, we'll actually generate the expr via `Mk::Block`
                // See the `Mk::Block` call in Desugar.cc's `buildMethod()` for reference.
            }

            Translator methodContext = this->enterMethodDef(isSingletonMethod, declLoc, name, enclosingBlockParamName);

            unique_ptr<parser::Node> body;
            if (defNode->body != nullptr) {
                if (PM_NODE_TYPE_P(defNode->body, PM_BEGIN_NODE)) {
                    auto beginNode = down_cast<pm_begin_node>(defNode->body);
                    auto statements = this->translateEnsure(beginNode);

                    // Prism uses a PM_BEGIN_NODE to model the body of a method that has a top level rescue/ensure, e.g.
                    //
                    //     def method_with_top_level_rescue
                    //       "body"
                    //     rescue
                    //       "fallback"
                    //     end
                    //
                    // This gets parsed as-if the body had an explicit begin/rescue/ensure block nested in it.
                    //
                    // This would cause the legacy parse tree to have an extra `parser::Kwbegin` node wrapping the body.
                    // To match the legacy parse tree, we dig into the `beginNode` and pull out its statements,
                    // skipping the creation of that parent `parser::Kwbegin` node.
                    if (statements.size() == 1) {
                        body = move(statements[0]);
                    } else {
                        if (!directlyDesugar || !hasExpr(statements)) {
                            body = make_unique<parser::Kwbegin>(location, move(statements));
                        } else {
                            auto args = nodeVecToStore<ast::InsSeq::STATS_store>(statements);
                            auto finalExpr = move(args.back());
                            args.pop_back();
                            auto expr = MK::InsSeq(location, move(args), move(finalExpr));
                            body = make_node_with_expr<parser::Kwbegin>(move(expr), location, move(statements));
                        }
                    }

                } else {
                    body = methodContext.translate(defNode->body);
                }
            }

            // Method defs are complex, and we're building support for different kinds of arguments bit by
            // bit. This bool is true when this particular method def is supported by our desugar logic.
            auto attemptToDesugarParams = directlyDesugar && hasExpr(receiver, body);

            ast::MethodDef::PARAMS_store paramsStore;
            ast::InsSeq::STATS_store statsStore;
            bool didDesugarParams = false; // ...and by impliciation, everything else (see `attemptToDesugarParams`)
            if (params != nullptr) {
                std::tie(paramsStore, statsStore, didDesugarParams) =
                    desugarParametersNode(params->params, attemptToDesugarParams);
            } else {
                didDesugarParams = attemptToDesugarParams;
            }

            if (!didDesugarParams) {
                if (isSingletonMethod) {
                    return make_unique<parser::DefS>(location, declLoc, move(receiver), name, move(params), move(body));
                } else {
                    return make_unique<parser::DefMethod>(location, declLoc, name, move(params), move(body));
                }
            }

            auto methodBody = body == nullptr ? MK::EmptyTree() : body->takeDesugaredExpr();

            auto methodExpr = MK::Method(location, declLoc, name, move(paramsStore), move(methodBody));

            if (isSingletonMethod) {
                ast::cast_tree<ast::MethodDef>(methodExpr)->flags.isSelfMethod = true;
                return make_node_with_expr<parser::DefS>(move(methodExpr), location, declLoc, move(receiver), name,
                                                         move(params), move(body));
            }

            return make_node_with_expr<parser::DefMethod>(move(methodExpr), location, declLoc, name, move(params),
                                                          move(body));
        }
        case PM_DEFINED_NODE: {
            auto definedNode = down_cast<pm_defined_node>(node);

            auto arg = translate(definedNode->value);

            if (!directlyDesugar) {
                return make_unique<parser::Defined>(location.join(arg->loc), move(arg));
            }

            ENFORCE(arg != nullptr);
            ENFORCE(arg->hasDesugaredExpr());

            auto valueNode = definedNode->value;

            switch (PM_NODE_TYPE(valueNode)) {
                // Desugar `defined?(@ivar)` to `::Magic.defined_instance_var(:@ivar)`
                case PM_INSTANCE_VARIABLE_READ_NODE: {
                    auto ivarNode = down_cast<pm_instance_variable_read_node>(valueNode);
                    auto loc = translateLoc(valueNode->location);
                    auto name = translateConstantName(ivarNode->name);
                    auto sym = MK::Symbol(loc, name);

                    auto expr = MK::Send1(loc, MK::Magic(loc), core::Names::definedInstanceVar(),
                                          location.copyWithZeroLength(), move(sym));

                    return make_node_with_expr<parser::Defined>(move(expr), location.join(loc), move(arg));
                }

                // Desugar `defined?(@@cvar)` to `::Magic.defined_instance_var(:@@cvar)`
                case PM_CLASS_VARIABLE_READ_NODE: {
                    auto cvarNode = down_cast<pm_class_variable_read_node>(valueNode);
                    auto loc = translateLoc(valueNode->location);
                    auto name = translateConstantName(cvarNode->name);
                    auto sym = MK::Symbol(loc, name);

                    auto expr = MK::Send1(loc, MK::Magic(loc), core::Names::definedClassVar(),
                                          location.copyWithZeroLength(), move(sym));

                    return make_node_with_expr<parser::Defined>(move(expr), location.join(loc), move(arg));
                }

                // Desugar `defined?(A::B::C)` to `::Magic.defined_p("A", "B", "C")`
                // or `defined?(::A::B::C)` to `::Magic.defined_p()`
                case PM_CONSTANT_READ_NODE:
                case PM_CONSTANT_PATH_NODE: {
                    ast::Send::ARGS_store args;
                    auto current = valueNode;

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
                    auto expr = MK::Send(arg->loc, MK::Magic(arg->loc), core::Names::defined_p(),
                                         location.copyWithZeroLength(), args.size(), move(args));
                    return make_node_with_expr<parser::Defined>(move(expr), location.join(arg->loc), move(arg));
                }
                default: {
                    // All other cases desugar to `::Magic.defined?()` with 0 arguments
                    ast::Send::ARGS_store args;
                    auto expr = MK::Send(arg->loc, MK::Magic(arg->loc), core::Names::defined_p(),
                                         location.copyWithZeroLength(), args.size(), move(args));
                    return make_node_with_expr<parser::Defined>(move(expr), location.join(arg->loc), move(arg));
                }
            }
        }
        case PM_ELSE_NODE: { // An `else` clauses, which can pertain to an `if`, `begin`, `case`, etc.
            auto elseNode = down_cast<pm_else_node>(node);
            return translate(up_cast(elseNode->statements));
        }
        case PM_EMBEDDED_STATEMENTS_NODE: { // Statements interpolated into a string.
            // e.g. the `#{bar}` in `"foo #{bar} baz"`
            // Can be multiple statements separated by `;`.
            auto embeddedStmtsNode = down_cast<pm_embedded_statements_node>(node);

            auto stmtsNode = embeddedStmtsNode->statements;
            if (stmtsNode == nullptr) {
                return make_node_with_expr<parser::Begin>(MK::Nil(location), location, NodeVec{});
            }

            auto inlineIfSingle = false;
            return translateStatements(stmtsNode, inlineIfSingle, location);
        }
        case PM_EMBEDDED_VARIABLE_NODE: {
            auto embeddedVariableNode = down_cast<pm_embedded_variable_node>(node);
            return translate(embeddedVariableNode->variable);
        }
        case PM_ENSURE_NODE: { // An `ensure` clause, which can pertain to a `begin`
            unreachable("PM_ENSURE_NODE is handled separately as part of PM_BEGIN_NODE, see its docs for details.");
        }
        case PM_FALSE_NODE: { // The `false` keyword
            return make_node_with_expr<parser::False>(MK::False(location), location);
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

            return make_node_with_expr<parser::Float>(MK::Float(location, val), location, move(valueString));
        }
        case PM_FLIP_FLOP_NODE: { // A flip-flop pattern, like the `flip..flop` in `if flip..flop`
            auto flipFlopNode = down_cast<pm_flip_flop_node>(node);

            auto left = patternTranslate(flipFlopNode->left);
            auto right = patternTranslate(flipFlopNode->right);

            if (PM_NODE_FLAG_P(flipFlopNode, PM_RANGE_FLAGS_EXCLUDE_END)) { // 3 dots: `flip...flop`
                return make_unsupported_node<parser::EFlipflop>(location, move(left), move(right));
            } else { // 2 dots: `flip..flop`
                return make_unsupported_node<parser::IFlipflop>(location, move(left), move(right));
            }
        }
        case PM_FOR_NODE: { // `for x in a; ...; end`
            auto forNode = down_cast<pm_for_node>(node);

            auto variable = translate(forNode->index);
            auto collection = translate(forNode->collection);
            auto body = translateStatements(forNode->statements);

            if (!directlyDesugar || !hasExpr(variable, collection, body)) {
                return make_unique<parser::For>(location, move(variable), move(collection), move(body));
            }

            // Desugar `for x in collection; body; end` into `collection.each { |x| body }`
            bool canProvideNiceDesugar = true;
            auto *mlhs = parser::NodeWithExpr::cast_node<parser::Mlhs>(variable.get());

            // Check if the variable is a simple local variable or a multi-target with only local variables
            if (mlhs) {
                // Multi-target: check if all are local variables (no nested Mlhs or other complex targets)
                canProvideNiceDesugar = absl::c_all_of(mlhs->exprs, [](const auto &c) {
                    return parser::NodeWithExpr::isa_node<parser::LVarLhs>(c.get());
                });
            } else {
                // Single variable: check if it's a local variable
                canProvideNiceDesugar = parser::NodeWithExpr::isa_node<parser::LVarLhs>(variable.get());
            }

            auto bodyExpr = body ? body->takeDesugaredExpr() : MK::EmptyTree();
            auto collectionExpr = collection->takeDesugaredExpr();
            auto locZeroLen = location.copyWithZeroLength();
            ast::MethodDef::PARAMS_store params;

            if (canProvideNiceDesugar) {
                // Simple case: `for x in a; body; end` -> `a.each { |x| body }`
                if (mlhs) {
                    for (auto &c : mlhs->exprs) {
                        params.emplace_back(c->takeDesugaredExpr());
                    }
                } else {
                    params.emplace_back(variable->takeDesugaredExpr());
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
                    masgnExpr = MK::Assign(location, variable->takeDesugaredExpr(), move(tempLocal));
                }

                bodyExpr = MK::InsSeq1(location, move(masgnExpr), move(bodyExpr));
            }

            auto block = MK::Block(location, move(bodyExpr), move(params));
            auto expr = MK::Send0Block(location, move(collectionExpr), core::Names::each(), locZeroLen, move(block));

            return make_node_with_expr<parser::For>(move(expr), location, move(variable), move(collection), move(body));
        }
        case PM_FORWARDING_ARGUMENTS_NODE: { // The `...` argument in a method call, like `foo(...)`
            return make_unique<parser::ForwardedArgs>(location);
        }
        case PM_FORWARDING_PARAMETER_NODE: { // The `...` parameter in a method definition, like `def foo(...)`
            // Desugared in desugarParametersNode().
            return make_unique<parser::ForwardArg>(location);
        }
        case PM_FORWARDING_SUPER_NODE: { // A `super` with no explicit arguments
            // It might have a literal block argument, though.

            auto forwardingSuperNode = down_cast<pm_forwarding_super_node>(node);

            // There's no `keyword_loc` field, so we make it ourselves from the start location.
            constexpr uint32_t length = "super"sv.size();
            auto keywordLoc = translateLoc(node->location.start, node->location.start + length);

            auto expr = MK::ZSuper(location, maybeTypedSuper());
            auto translatedNode = make_node_with_expr<parser::ZSuper>(move(expr), keywordLoc);

            auto blockArgumentNode = forwardingSuperNode->block;

            if (blockArgumentNode != nullptr) { // always a PM_BLOCK_NODE
                return translateCallWithBlock(up_cast(blockArgumentNode), move(translatedNode));
            }

            return translatedNode;
        }
        case PM_GLOBAL_VARIABLE_AND_WRITE_NODE: { // And-assignment to a global variable, e.g. `$g &&= false`
            return translateVariableAssignment<pm_global_variable_and_write_node, parser::AndAsgn, parser::GVarLhs>(
                node, location);
        }
        case PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a global variable, e.g. `$g += 1`
            return translateVariableAssignment<pm_global_variable_operator_write_node, parser::OpAsgn, parser::GVarLhs>(
                node, location);
        }
        case PM_GLOBAL_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a global variable, e.g. `$g ||= true`
            return translateVariableAssignment<pm_global_variable_or_write_node, parser::OrAsgn, parser::GVarLhs>(
                node, location);
        }
        case PM_GLOBAL_VARIABLE_READ_NODE: { // A global variable, like `$g`
            auto globalVarReadNode = down_cast<pm_global_variable_read_node>(node);
            auto name = translateConstantName(globalVarReadNode->name);
            auto expr = ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Global, name);

            return make_node_with_expr<parser::GVar>(move(expr), location, name);
        }
        case PM_GLOBAL_VARIABLE_TARGET_NODE: { // Target of an indirect write to a global variable
            // ... like `$target1, $target2 = 1, 2`, `rescue => $target`, etc.
            auto globalVariableTargetNode = down_cast<pm_global_variable_target_node>(node);
            auto name = translateConstantName(globalVariableTargetNode->name);
            auto expr = ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Global, name);

            return make_node_with_expr<parser::GVarLhs>(move(expr), location, name);
        }
        case PM_GLOBAL_VARIABLE_WRITE_NODE: { // Regular assignment to a global variable, e.g. `$g = 1`
            return translateAssignment<pm_global_variable_write_node, parser::GVarLhs>(node);
        }
        case PM_HASH_NODE: { // A hash literal, like `{ a: 1, b: 2 }`
            auto hashNode = down_cast<pm_hash_node>(node);

            auto kvPairs = translateKeyValuePairs(hashNode->elements);

            auto elementsHaveExprs = absl::c_all_of(kvPairs, [](const auto &node) {
                // `parser::Pair` nodes never have a desugared expr, because they have no ExpressionPtr equivalent.
                // Instead, we check their children ourselves.
                if (auto *pair = parser::NodeWithExpr::cast_node<parser::Pair>(node.get())) {
                    return hasExpr(pair->key, pair->value);
                }

                return hasExpr(node);
            });

            if (!directlyDesugar || !elementsHaveExprs) {
                return make_unique<parser::Hash>(location, false, move(kvPairs));
            }

            auto hashExpr = desugarHash(location, kvPairs);

            return make_node_with_expr<parser::Hash>(move(hashExpr), location, false, move(kvPairs));
        }
        case PM_IF_NODE: { // An `if` statement or modifier, like `if cond; ...; end` or `a.b if cond`
            auto ifNode = down_cast<pm_if_node>(node);

            auto predicate = translate(ifNode->predicate);
            auto ifTrue = translate(up_cast(ifNode->statements));
            auto ifFalse = translate(ifNode->subsequent);

            return translateIfNode(location, move(predicate), move(ifTrue), move(ifFalse));
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
                auto complexNode = make_unique<parser::Complex>(numberLoc, string(value));
                core::NameRef unaryOp = (sign == '-') ? core::Names::unaryMinus() : core::Names::unaryPlus();

                auto unarySend = MK::Send0(location, move(complexCall), unaryOp,
                                           core::LocOffsets{location.beginLoc, numberLoc.beginLoc});

                return make_node_with_expr<parser::Send>(move(unarySend), location, move(complexNode), unaryOp,
                                                         core::LocOffsets{location.beginLoc, location.beginLoc + 1},
                                                         NodeVec{});
            }

            // No leading sign; return the Complex node directly
            return make_node_with_expr<parser::Complex>(move(complexCall), location, string(value));
        }
        case PM_IMPLICIT_NODE: { // A hash key without explicit value, like the `k4` in `{ k4: }`
            auto implicitNode = down_cast<pm_implicit_node>(node);
            return translate(implicitNode->value);
        }
        case PM_IMPLICIT_REST_NODE: { // An implicit splat, like the `,` in `a, = 1, 2, 3`
            auto restLoc = core::LocOffsets{location.beginLoc + 1, location.beginLoc + 1};
            core::NameRef sorbetName = core::Names::restargs();
            auto expr = MK::RestParam(restLoc, MK::Local(restLoc, sorbetName));

            return make_node_with_expr<parser::RestParam>(move(expr), restLoc, sorbetName, restLoc);
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
            auto receiver = translate(indexedTargetNode->receiver);
            auto arguments = translateArguments(indexedTargetNode->arguments, up_cast(indexedTargetNode->block));

            if (!directlyDesugar || !hasExpr(receiver, arguments)) {
                return make_unique<parser::Send>(location, move(receiver), core::Names::squareBracketsEq(), lBracketLoc,
                                                 move(arguments));
            }

            // Build the arguments for the Send expression
            ast::Send::ARGS_store argExprs;
            argExprs.reserve(arguments.size());
            for (auto &arg : arguments) {
                argExprs.emplace_back(arg->takeDesugaredExpr());
            }

            auto expr = MK::Send(location, receiver->takeDesugaredExpr(), core::Names::squareBracketsEq(), lBracketLoc,
                                 argExprs.size(), move(argExprs));
            return make_node_with_expr<parser::Send>(move(expr), location, move(receiver),
                                                     core::Names::squareBracketsEq(), lBracketLoc, move(arguments));
        }
        case PM_INSTANCE_VARIABLE_AND_WRITE_NODE: { // And-assignment to an instance variable, e.g. `@iv &&= false`
            return translateVariableAssignment<pm_instance_variable_and_write_node, parser::AndAsgn, parser::IVarLhs>(
                node, location);
        }
        case PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to an instance variable, e.g. `@iv += 1`
            return translateVariableAssignment<pm_instance_variable_operator_write_node, parser::OpAsgn,
                                               parser::IVarLhs>(node, location);
        }
        case PM_INSTANCE_VARIABLE_OR_WRITE_NODE: { // Or-assignment to an instance variable, e.g. `@iv ||= true`
            return translateVariableAssignment<pm_instance_variable_or_write_node, parser::OrAsgn, parser::IVarLhs>(
                node, location);
        }
        case PM_INSTANCE_VARIABLE_READ_NODE: { // An instance variable, like `@iv`
            auto instanceVarNode = down_cast<pm_instance_variable_read_node>(node);
            auto name = translateConstantName(instanceVarNode->name);
            auto expr =
                ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Instance, name);

            return make_node_with_expr<parser::IVar>(move(expr), location, name);
        }
        case PM_INSTANCE_VARIABLE_TARGET_NODE: { // Target of an indirect write to an instance variable
            // ... like `@target1, @target2 = 1, 2`, `rescue => @target`, etc.
            auto instanceVariableTargetNode = down_cast<pm_instance_variable_target_node>(node);
            auto name = translateConstantName(instanceVariableTargetNode->name);
            auto expr =
                ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Instance, name);

            return make_node_with_expr<parser::IVarLhs>(move(expr), location, name);
        }
        case PM_INSTANCE_VARIABLE_WRITE_NODE: { // Regular assignment to an instance variable, e.g. `@iv = 1`
            return translateAssignment<pm_instance_variable_write_node, parser::IVarLhs>(node);
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

            return make_node_with_expr<parser::Integer>(MK::Int(location, val), location, move(valueString));
        }
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE: { // An interpolated regex literal in a conditional...
            // ...that implicitly checks against the last read line by an IO object, e.g. `if /wat #{123}/`
            auto interpolatedMatchLastLineNode = down_cast<pm_interpolated_match_last_line_node>(node);

            auto parts = translateMulti(interpolatedMatchLastLineNode->parts);
            auto options = translateRegexpOptions(interpolatedMatchLastLineNode->closing_loc);
            auto regex = make_unique<parser::Regexp>(location, move(parts), move(options));

            return make_unsupported_node<parser::MatchCurLine>(location, move(regex));
        }
        case PM_INTERPOLATED_REGULAR_EXPRESSION_NODE: { // A regular expression with interpolation, like `/a #{b} c/`
            auto interpolatedRegexNode = down_cast<pm_interpolated_regular_expression_node>(node);

            auto parts = translateMulti(interpolatedRegexNode->parts);
            auto options = translateRegexpOptions(interpolatedRegexNode->closing_loc);

            if (!directlyDesugar || !hasExpr(parts)) {
                return make_unique<parser::Regexp>(location, move(parts), move(options));
            }

            // Desugar interpolated regexp to Regexp.new(pattern, options)
            auto pattern = desugarDString(location, interpolatedRegexNode->parts);
            auto optsExpr = options->takeDesugaredExpr();

            auto cnst = MK::Constant(location, core::Symbols::Regexp());
            auto expr = MK::Send2(location, move(cnst), core::Names::new_(), location.copyWithZeroLength(),
                                  move(pattern), move(optsExpr));

            return make_node_with_expr<parser::Regexp>(move(expr), location, move(parts), move(options));
        }
        case PM_INTERPOLATED_STRING_NODE: { // An interpolated string like `"foo #{bar} baz"`
            auto interpolatedStringNode = down_cast<pm_interpolated_string_node>(node);

            auto sorbetParts = translateMulti(interpolatedStringNode->parts);

            if (!directlyDesugar || !hasExpr(sorbetParts)) {
                return make_unique<parser::DString>(location, move(sorbetParts));
            }

            // Desugar `"a #{b} c"` to `::Magic.<string-interpolate>("a ", b, " c")`
            auto desugared = desugarDString(location, interpolatedStringNode->parts);
            return make_node_with_expr<parser::DString>(move(desugared), location, move(sorbetParts));
        }
        case PM_INTERPOLATED_SYMBOL_NODE: { // A symbol like `:"a #{b} c"`
            auto interpolatedSymbolNode = down_cast<pm_interpolated_symbol_node>(node);

            auto sorbetParts = translateMulti(interpolatedSymbolNode->parts);

            if (!directlyDesugar || !hasExpr(sorbetParts)) {
                return make_unique<parser::DSymbol>(location, move(sorbetParts));
            }

            // Desugar `:"a #{b} c"` to `::Magic.<string-interpolate>("a ", b, " c").intern()`
            auto desugared = desugarDString(location, interpolatedSymbolNode->parts);
            auto interned = MK::Send0(location, move(desugared), core::Names::intern(), location.copyWithZeroLength());
            return make_node_with_expr<parser::DSymbol>(move(interned), location, move(sorbetParts));
        }
        case PM_INTERPOLATED_X_STRING_NODE: { // An executable string with backticks, like `echo "Hello, world!"`
            auto interpolatedXStringNode = down_cast<pm_interpolated_x_string_node>(node);

            auto sorbetParts = translateMulti(interpolatedXStringNode->parts);

            if (!directlyDesugar || !hasExpr(sorbetParts)) {
                return make_unique<parser::XString>(location, move(sorbetParts));
            }

            auto desugared = desugarDString(location, interpolatedXStringNode->parts);
            auto res = MK::Send1(location, MK::Self(location), core::Names::backtick(), location.copyWithZeroLength(),
                                 move(desugared));
            return make_node_with_expr<parser::XString>(move(res), location, move(sorbetParts));
        }
        case PM_IT_LOCAL_VARIABLE_READ_NODE: { // The `it` implicit parameter added in Ruby 3.4, e.g. `a.map { it + 1 }`
            [[fallthrough]];
        }
        case PM_IT_PARAMETERS_NODE: { // Used in a parameter list for lambdas that the `it` implicit parameter.
            // See Prism::ParserStorage::ParsedRubyVersion
            unreachable("The `it` keyword was introduced in Ruby 3.4, which isn't supported by Sorbet yet.");
        }
        case PM_KEYWORD_HASH_NODE: { // A hash of keyword arguments, like `foo(a: 1, b: 2)`
            auto keywordHashNode = down_cast<pm_keyword_hash_node>(node);

            auto kvPairs = translateKeyValuePairs(keywordHashNode->elements);

            auto elements = absl::MakeSpan(keywordHashNode->elements.nodes, keywordHashNode->elements.size);
            ENFORCE(!elements.empty());

            auto isKwargs =
                PM_NODE_FLAG_P(keywordHashNode, PM_KEYWORD_HASH_NODE_FLAGS_SYMBOL_KEYS) ||
                absl::c_all_of(absl::MakeSpan(keywordHashNode->elements.nodes, keywordHashNode->elements.size),
                               [](auto *node) {
                                   // Checks if the given node is a keyword hash element based on the standards of
                                   // Sorbet's legacy parser. Based on `Builder::isKeywordHashElement()`

                                   if (PM_NODE_TYPE_P(node, PM_ASSOC_NODE)) { // A regular key/value pair
                                       auto pair = down_cast<pm_assoc_node>(node);
                                       return pair->key && PM_NODE_TYPE_P(pair->key, PM_SYMBOL_NODE);
                                   }

                                   if (PM_NODE_TYPE_P(node, PM_ASSOC_SPLAT_NODE)) { // A `**h` or `**` kwarg splat
                                       return true;
                                   };

                                   return false;
                               });

            return make_unique<parser::Hash>(location, isKwargs, move(kvPairs));
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

            return make_node_with_expr<parser::Kwrestarg>(
                MK::RestParam(kwrestLoc, MK::KeywordArg(kwrestLoc, sorbetName)), kwrestLoc, sorbetName);
        }
        case PM_LAMBDA_NODE: { // lambda literals, like `-> { 123 }`
            auto lambdaNode = down_cast<pm_lambda_node>(node);

            auto receiver = make_unique<parser::Const>(location, nullptr, core::Names::Constants::Kernel());
            auto sendNode = make_unique<parser::Send>(location, move(receiver), core::Names::lambda(),
                                                      translateLoc(lambdaNode->operator_loc), NodeVec{});

            return translateCallWithBlock(node, move(sendNode));
        }
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE: { // And-assignment to a local variable, e.g. `local &&= false`
            return translateVariableAssignment<pm_local_variable_and_write_node, parser::AndAsgn, parser::LVarLhs>(
                node, location);
        }
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a local variable, e.g. `local += 1`
            return translateVariableAssignment<pm_local_variable_operator_write_node, parser::OpAsgn, parser::LVarLhs>(
                node, location);
        }
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a local variable, e.g. `local ||= true`
            return translateVariableAssignment<pm_local_variable_or_write_node, parser::OrAsgn, parser::LVarLhs>(
                node, location);
        }
        case PM_LOCAL_VARIABLE_READ_NODE: { // A local variable, like `lv`
            auto localVarReadNode = down_cast<pm_local_variable_read_node>(node);
            auto name = translateConstantName(localVarReadNode->name);
            ast::ExpressionPtr expr = MK::Local(location, name);

            return make_node_with_expr<parser::LVar>(move(expr), location, name);
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // Target of an indirect write to a local variable
            // ... like `target1, target2 = 1, 2`, `rescue => target`, etc.
            auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
            auto name = translateConstantName(localVarTargetNode->name);
            auto expr = MK::Local(location, name);

            return make_node_with_expr<parser::LVarLhs>(move(expr), location, name);
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: { // Regular assignment to a local variable, e.g. `local = 1`
            return translateAssignment<pm_local_variable_write_node, parser::LVarLhs>(node);
        }
        case PM_MATCH_LAST_LINE_NODE: { // A regex literal in a conditional...
            // ...that implicitly checks against the last read line by an IO object, e.g. `if /wat/`
            auto matchLastLineNode = down_cast<pm_match_last_line_node>(node);

            auto contentLoc = translateLoc(matchLastLineNode->content_loc);

            auto regex =
                translateRegexp(location, contentLoc, matchLastLineNode->unescaped, matchLastLineNode->closing_loc);

            return make_unsupported_node<parser::MatchCurLine>(location, move(regex));
        }
        case PM_MATCH_REQUIRED_NODE: {
            auto matchRequiredNode = down_cast<pm_match_required_node>(node);

            auto value = patternTranslate(matchRequiredNode->value);
            auto pattern = patternTranslate(matchRequiredNode->pattern);

            return make_unique<parser::MatchPattern>(location, move(value), move(pattern));
        }
        case PM_MATCH_PREDICATE_NODE: {
            auto matchPredicateNode = down_cast<pm_match_predicate_node>(node);

            auto value = patternTranslate(matchPredicateNode->value);
            auto pattern = patternTranslate(matchPredicateNode->pattern);

            return make_unique<parser::MatchPatternP>(location, move(value), move(pattern));
        }
        case PM_MATCH_WRITE_NODE: { // A regex match that assigns to a local variable, like `a =~ /wat/`
            auto matchWriteNode = down_cast<pm_match_write_node>(node);

            // "Match writes" let you bind regex capture groups directly into new variables.
            // Sorbet doesn't treat this syntax in a special way, so it doesn't know that it introduces new local var.
            // It's treated as a normal call to `=~` with a Regexp receiver and the rhs as an argument.
            //
            // This is why we just translate the `call` and completely ignore `matchWriteNode->targets`.

            return translate(up_cast(matchWriteNode->call));
        }
        case PM_MODULE_NODE: { // Modules declarations, like `module A::B::C; ...; end`
            auto moduleNode = down_cast<pm_module_node>(node);

            auto name = translate(moduleNode->constant_path);
            auto declLoc = translateLoc(moduleNode->module_keyword_loc).join(name->loc);
            auto body = this->enterModuleContext().translate(moduleNode->body);

            if (!directlyDesugar || !hasExpr(name, body)) {
                return make_unique<parser::Module>(location, declLoc, move(name), move(body));
            }

            auto bodyExprsOpt = desugarScopeBodyToRHSStore(moduleNode->body, body);
            if (!bodyExprsOpt.has_value()) {
                return make_unique<parser::Module>(location, declLoc, move(name), move(body));
            }
            auto bodyExprs = move(*bodyExprsOpt);

            auto nameExpr = name->takeDesugaredExpr();
            auto moduleDef = MK::Module(location, declLoc, move(nameExpr), move(bodyExprs));

            return make_node_with_expr<parser::Module>(move(moduleDef), location, declLoc, move(name), move(body));
        }
        case PM_MULTI_TARGET_NODE: { // A multi-target like the `(x2, y2)` in `p1, (x2, y2) = a`
            auto multiTargetNode = down_cast<pm_multi_target_node>(node);

            auto lhsLoc = translateLoc(mlhsLocation(multiTargetNode));

            return translateMultiTargetLhs(multiTargetNode, lhsLoc);
        }
        case PM_MULTI_WRITE_NODE: { // Multi-assignment, like `a, b = 1, 2`
            auto multiWriteNode = down_cast<pm_multi_write_node>(node);

            auto lhsLoc = translateLoc(mlhsLocation(multiWriteNode));

            auto multiLhsNode = translateMultiTargetLhs(multiWriteNode, lhsLoc);
            auto rhsValue = translate(multiWriteNode->value);

            // Sorbet's legacy parser doesn't include the opening `(` (see `startLoc()` for details),
            // so we can't just use the entire Prism location for the Masgn node.
            location = translateLoc(startLoc(up_cast(multiWriteNode)), endLoc(multiWriteNode->value));

            if (!directlyDesugar || !hasExpr(rhsValue, multiLhsNode->exprs)) {
                return make_unique<parser::Masgn>(location, move(multiLhsNode), move(rhsValue));
            }

            auto rhsExpr = rhsValue->takeDesugaredExpr();
            auto expr = desugarMlhs(location, multiLhsNode.get(), move(rhsExpr));
            return make_node_with_expr<parser::Masgn>(move(expr), location, move(multiLhsNode), move(rhsValue));
        }
        case PM_NEXT_NODE: { // A `next` statement, e.g. `next`, `next 1, 2, 3`
            auto nextNode = down_cast<pm_next_node>(node);

            auto arguments = translateArguments(nextNode->arguments);

            if (arguments.empty()) {
                auto expr = MK::Next(location, MK::EmptyTree());
                return make_node_with_expr<parser::Next>(move(expr), location, move(arguments));
            }

            if (!directlyDesugar || !hasExpr(arguments)) {
                return make_unique<parser::Next>(location, move(arguments));
            }

            ExpressionPtr nextArgs;
            if (arguments.size() == 1) {
                auto &first = arguments[0];
                nextArgs = first == nullptr ? MK::EmptyTree() : first->takeDesugaredExpr();
            } else {
                auto args = nodeVecToStore<ast::Array::ENTRY_store>(arguments);
                auto arrayLocation = parser.translateLocation(nextNode->arguments->base.location);
                nextArgs = MK::Array(arrayLocation, std::move(args));
            }
            auto expr = MK::Next(location, move(nextArgs));
            return make_node_with_expr<parser::Next>(move(expr), location, move(arguments));
        }
        case PM_NIL_NODE: { // The `nil` keyword
            return make_node_with_expr<parser::Nil>(MK::Nil(location), location);
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
            auto expr = ast::make_expression<ast::UnresolvedIdent>(location, ast::UnresolvedIdent::Kind::Global, name);

            return make_node_with_expr<parser::NthRef>(move(expr), location, number);
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: { // An optional keyword parameter, like `def foo(a: 1)`
            auto optionalKeywordParamNode = down_cast<pm_optional_keyword_parameter_node>(node);
            auto nameLoc = translateLoc(optionalKeywordParamNode->name_loc);

            auto name = translateConstantName(optionalKeywordParamNode->name);
            auto value = translate(optionalKeywordParamNode->value);

            if (!hasExpr(value)) {
                return make_unique<parser::Kwoptarg>(location, name, nameLoc, move(value));
            }

            auto expr = MK::OptionalParam(location, MK::KeywordArg(nameLoc, name), value->takeDesugaredExpr());
            return make_node_with_expr<parser::Kwoptarg>(move(expr), location, name, nameLoc, move(value));
        }
        case PM_OPTIONAL_PARAMETER_NODE: { // An optional positional parameter, like `def foo(a = 1)`
            auto optionalParamNode = down_cast<pm_optional_parameter_node>(node);
            auto nameLoc = translateLoc(optionalParamNode->name_loc);

            auto name = translateConstantName(optionalParamNode->name);
            auto value = translate(optionalParamNode->value);

            if (!hasExpr(value)) {
                return make_unique<parser::OptParam>(location, name, nameLoc, move(value));
            }

            auto expr = MK::OptionalParam(location, MK::Local(nameLoc, name), value->takeDesugaredExpr());
            return make_node_with_expr<parser::OptParam>(move(expr), location, name, nameLoc, move(value));
        }
        case PM_OR_NODE: { // operator `||` and `or`
            auto orNode = down_cast<pm_or_node>(node);

            auto left = translate(orNode->left);
            auto right = translate(orNode->right);

            if (!directlyDesugar || !hasExpr(left, right)) {
                return make_unique<parser::Or>(location, move(left), move(right));
            }

            auto lhsExpr = left->takeDesugaredExpr();
            auto rhsExpr = right->takeDesugaredExpr();

            if (preserveConcreteSyntax) {
                auto orOrLoc = core::LocOffsets{left->loc.endPos(), right->loc.beginPos()};
                auto magicSend = MK::Send2(location, MK::Magic(location.copyWithZeroLength()), core::Names::orOr(),
                                           orOrLoc, move(lhsExpr), move(rhsExpr));
                return make_node_with_expr<parser::Or>(move(magicSend), location, move(left), move(right));
            }

            if (isa_reference(lhsExpr)) {
                auto cond = MK::cpRef(lhsExpr);
                auto if_ = MK::If(location, move(cond), move(lhsExpr), move(rhsExpr));
                return make_node_with_expr<parser::Or>(move(if_), location, move(left), move(right));
            }

            // For non-reference expressions, create a temporary variable so we don't evaluate the LHS twice.
            // E.g. `x = 1 || 2` becomes `x = (temp = 1; temp ? temp : 2)`
            core::NameRef tempLocalName = nextUniqueDesugarName(core::Names::orOr());
            auto lhsLoc = left->loc;
            auto rhsLoc = right->loc;
            auto condLoc =
                lhsLoc.exists() && rhsLoc.exists() ? core::LocOffsets{lhsLoc.endPos(), rhsLoc.beginPos()} : lhsLoc;
            auto tempAssign = MK::Assign(location, tempLocalName, move(lhsExpr));
            auto cond = MK::Local(condLoc, tempLocalName);
            auto thenp = MK::Local(lhsLoc, tempLocalName);
            auto if_ = MK::If(location, move(cond), move(thenp), move(rhsExpr));
            auto wrapped = MK::InsSeq1(location, move(tempAssign), move(if_));
            return make_node_with_expr<parser::Or>(move(wrapped), location, move(left), move(right));
        }
        case PM_PARAMETERS_NODE: { // The parameters declared at the top of a PM_DEF_NODE
            unreachable("PM_PARAMETERS_NODE is handled separately in translateParametersNode.");
        }
        case PM_PARENTHESES_NODE: { // A parethesized expression, e.g. `(a)`
            auto parensNode = down_cast<pm_parentheses_node>(node);

            auto stmtsNode = parensNode->body;

            if (stmtsNode == nullptr) {
                return make_node_with_expr<parser::Begin>(MK::Nil(location), location, NodeVec{});
            }

            if (PM_NODE_TYPE_P(stmtsNode, PM_STATEMENTS_NODE)) {
                auto inlineIfSingle = false;
                // Override the begin node location to be the parentheses location instead of the statements location
                return translateStatements(down_cast<pm_statements_node>(stmtsNode), inlineIfSingle, location);
            } else {
                return translate(stmtsNode);
            }
        }
        case PM_PRE_EXECUTION_NODE: {
            auto preExecutionNode = down_cast<pm_pre_execution_node>(node);
            auto body = translateStatements(preExecutionNode->statements);
            return make_unsupported_node<parser::Preexe>(location, move(body));
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

            return translate(up_cast(programNode->statements));
        }
        case PM_POST_EXECUTION_NODE: {
            auto postExecutionNode = down_cast<pm_post_execution_node>(node);
            auto body = translateStatements(postExecutionNode->statements);
            return make_unsupported_node<parser::Postexe>(location, move(body));
        }
        case PM_RANGE_NODE: { // A Range literal, e.g. `a..b`, `a..`, `..b`, `a...b`, `a...`, `...b`
            auto rangeNode = down_cast<pm_range_node>(node);

            auto left = translate(rangeNode->left);
            auto right = translate(rangeNode->right);

            bool isExclusive = PM_NODE_FLAG_P(rangeNode, PM_RANGE_FLAGS_EXCLUDE_END);

            if (!directlyDesugar || !hasExpr(left, right)) {
                if (isExclusive) { // `...`
                    return make_unique<parser::ERange>(location, move(left), move(right));
                } else { // `..`
                    return make_unique<parser::IRange>(location, move(left), move(right));
                }
            }

            auto recv = MK::Magic(location);
            auto locZeroLen = core::LocOffsets{location.beginPos(), location.beginPos()};

            auto fromExpr = left ? left->takeDesugaredExpr() : MK::EmptyTree();
            auto toExpr = right ? right->takeDesugaredExpr() : MK::EmptyTree();

            auto excludeEndExpr = isExclusive ? MK::True(location) : MK::False(location);

            // Desugar to `::Kernel.<buildRange>(from, to, excludeEnd)`
            auto desugaredExpr = MK::Send3(location, move(recv), core::Names::buildRange(), locZeroLen, move(fromExpr),
                                           move(toExpr), move(excludeEndExpr));

            if (isExclusive) { // `...`
                return make_node_with_expr<parser::ERange>(move(desugaredExpr), location, move(left), move(right));
            } else { // `..`
                return make_node_with_expr<parser::IRange>(move(desugaredExpr), location, move(left), move(right));
            }
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
            auto send = MK::Send1(location, move(kernel), rationalName, location.copyWithZeroLength(),
                                  MK::String(location, valueName));

            return make_node_with_expr<parser::Rational>(move(send), location, value);
        }
        case PM_REDO_NODE: { // The `redo` keyword
            return make_unsupported_node<parser::Redo>(location);
        }
        case PM_REGULAR_EXPRESSION_NODE: { // A regular expression literal, e.g. `/foo/`
            auto regexNode = down_cast<pm_regular_expression_node>(node);

            auto contentLoc = translateLoc(regexNode->content_loc);

            return translateRegexp(location, contentLoc, regexNode->unescaped, regexNode->closing_loc);
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: { // A required keyword parameter, like `def foo(a:)`
            auto requiredKeywordParamNode = down_cast<pm_required_keyword_parameter_node>(node);
            auto name = translateConstantName(requiredKeywordParamNode->name);

            return make_node_with_expr<parser::Kwarg>(MK::KeywordArg(location, name), location, name);
        }
        case PM_REQUIRED_PARAMETER_NODE: { // A required positional parameter, like `def foo(a)`
            auto requiredParamNode = down_cast<pm_required_parameter_node>(node);
            auto name = translateConstantName(requiredParamNode->name);
            auto expr = MK::Local(location, name);

            return make_node_with_expr<parser::Param>(move(expr), location, name);
        }
        case PM_RESCUE_MODIFIER_NODE: {
            auto rescueModifierNode = down_cast<pm_rescue_modifier_node>(node);
            auto body = translate(rescueModifierNode->expression);
            auto rescue = translate(rescueModifierNode->rescue_expression);
            auto keywordLoc = translateLoc(rescueModifierNode->keyword_loc);

            auto resbodyLoc = core::LocOffsets{keywordLoc.beginPos(), location.endPos()};

            if (!directlyDesugar || !hasExpr(body, rescue)) {
                // In rescue modifiers, users can't specify exception classes or names, so they're always null.
                std::unique_ptr<Node> rescuedExceptions = nullptr;
                auto resBody = make_unique<parser::Resbody>(resbodyLoc, move(rescuedExceptions), nullptr, move(rescue));

                auto cases = NodeVec1(move(resBody));
                return make_unique<parser::Rescue>(location, move(body), move(cases), nullptr);
            }

            auto bodyExpr = body->takeDesugaredExpr();
            auto rescueExpr = rescue->takeDesugaredExpr();

            // Create a RescueCase with empty exceptions and a <rescueTemp> variable
            ast::RescueCase::EXCEPTION_store exceptions;
            auto rescueTemp = nextUniqueDesugarName(core::Names::rescueTemp());

            auto rescueCaseLoc =
                translateLoc(rescueModifierNode->keyword_loc.start, rescueModifierNode->base.location.end);
            auto rescueCase = ast::make_expression<ast::RescueCase>(
                rescueCaseLoc, move(exceptions), ast::MK::Local(keywordLoc, rescueTemp), move(rescueExpr));

            ast::Rescue::RESCUE_CASE_store rescueCases;
            rescueCases.emplace_back(move(rescueCase));
            auto expr = ast::make_expression<ast::Rescue>(location, move(bodyExpr), move(rescueCases),
                                                          ast::MK::EmptyTree(), ast::MK::EmptyTree());

            auto cases = NodeVec1(make_unique<parser::Resbody>(resbodyLoc, nullptr, nullptr, move(rescue)));

            return make_node_with_expr<parser::Rescue>(move(expr), location, move(body), move(cases), nullptr);
        }
        case PM_RESCUE_NODE: {
            unreachable("PM_RESCUE_NODE is handled separately in translateRescue, see its docs for details.");
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

            auto expr = MK::RestParam(location, MK::Local(nameLoc, sorbetName));
            return make_node_with_expr<parser::RestParam>(move(expr), location, sorbetName, nameLoc);
        }
        case PM_RETURN_NODE: { // A `return` statement, like `return 1, 2, 3`
            auto returnNode = down_cast<pm_return_node>(node);

            auto returnValues = translateArguments(returnNode->arguments);

            if (returnValues.empty()) {
                auto expr = MK::Return(location, MK::EmptyTree());
                return make_node_with_expr<parser::Return>(move(expr), location, move(returnValues));
            }

            if (!directlyDesugar || !hasExpr(returnValues)) {
                return make_unique<parser::Return>(location, move(returnValues));
            }

            ExpressionPtr returnArgs;
            if (returnValues.size() == 1) {
                auto &first = returnValues[0];
                returnArgs = first == nullptr ? MK::EmptyTree() : first->takeDesugaredExpr();
            } else {
                auto args = nodeVecToStore<ast::Array::ENTRY_store>(std::move(returnValues));
                auto arrayLocation = parser.translateLocation(returnNode->arguments->base.location);
                returnArgs = MK::Array(arrayLocation, std::move(args));
            }
            auto expr = MK::Return(location, std::move(returnArgs));
            return make_node_with_expr<parser::Return>(move(expr), location, std::move(returnValues));
        }
        case PM_RETRY_NODE: { // The `retry` keyword
            return make_node_with_expr<parser::Retry>(ast::make_expression<ast::Retry>(location), location);
        }
        case PM_SELF_NODE: { // The `self` keyword
            return make_node_with_expr<parser::Self>(MK::Self(location), location);
        }
        case PM_SHAREABLE_CONSTANT_NODE: {
            // Sorbet doesn't handle `shareable_constant_value` yet (https://bugs.ruby-lang.org/issues/17273).
            // We'll just handle the inner constant assignment as normal.
            auto shareableConstantNode = down_cast<pm_shareable_constant_node>(node);
            return translate(shareableConstantNode->write);
        }
        case PM_SINGLETON_CLASS_NODE: { // A singleton class, like `class << self ... end`
            auto classNode = down_cast<pm_singleton_class_node>(node);

            auto declLoc = translateLoc(classNode->class_keyword_loc);
            auto receiver = translate(classNode->expression); // The receiver like `self` in `class << self`
            auto body = this->enterClassContext().translate(classNode->body);

            if (!directlyDesugar || !hasExpr(receiver, body)) {
                return make_unique<parser::SClass>(location, declLoc, move(receiver), move(body));
            }

            if (!PM_NODE_TYPE_P(classNode->expression, PM_SELF_NODE)) {
                if (auto e = ctx.beginIndexerError(receiver->loc, core::errors::Desugar::InvalidSingletonDef)) {
                    e.setHeader("`{}` is only supported for `{}`", "class << EXPRESSION", "class << self");
                }
                auto emptyTree = MK::EmptyTree();
                return make_node_with_expr<parser::SClass>(move(emptyTree), location, declLoc, move(receiver), nullptr);
            }

            auto bodyExprsOpt = desugarScopeBodyToRHSStore(classNode->body, body);
            if (!bodyExprsOpt.has_value()) {
                return make_unique<parser::SClass>(location, declLoc, move(receiver), move(body));
            }
            auto bodyExprs = move(*bodyExprsOpt);

            // Singleton classes are modelled as a class with a special name `<singleton>`
            auto singletonClassName = ast::make_expression<ast::UnresolvedIdent>(
                receiver->loc, ast::UnresolvedIdent::Kind::Class, core::Names::singleton());

            auto sClassDef = MK::Class(location, declLoc, move(singletonClassName), ast::ClassDef::ANCESTORS_store{},
                                       move(bodyExprs));

            return make_node_with_expr<parser::SClass>(move(sClassDef), location, declLoc, move(receiver), move(body));
        }
        case PM_SOURCE_ENCODING_NODE: { // The `__ENCODING__` keyword
            return make_node_with_expr<parser::EncodingLiteral>(
                MK::Send0(location, MK::Magic(location), core::Names::getEncoding(), location.copyWithZeroLength()),
                location);
        }
        case PM_SOURCE_FILE_NODE: { // The `__FILE__` keyword
            return make_node_with_expr<parser::FileLiteral>(MK::String(location, core::Names::currentFile()), location);
        }
        case PM_SOURCE_LINE_NODE: { // The `__LINE__` keyword
            auto details = ctx.locAt(location).toDetails(ctx);
            ENFORCE(details.first.line == details.second.line, "position corrupted");

            return make_node_with_expr<parser::LineLiteral>(MK::Int(location, details.first.line), location);
        }
        case PM_SPLAT_NODE: { // A splat, like `*a` in an array literal or method call
            auto splatNode = down_cast<pm_splat_node>(node);

            auto expr = translate(splatNode->expression);

            if (!directlyDesugar || !hasExpr(expr)) {
                if (expr == nullptr) { // An anonymous splat like `f(*)`
                    return make_unique<parser::ForwardedRestArg>(location);
                } else { // Splatting an expression like `f(*a)`
                    return make_unique<parser::Splat>(location, move(expr));
                }
            }

            if (expr == nullptr) { // An anonymous splat like `f(*)`
                auto var = MK::Local(location, core::Names::star());
                auto splatExpr = MK::Splat(location, move(var));
                return make_node_with_expr<parser::ForwardedRestArg>(move(splatExpr), location);
            } else { // Splatting an expression like `f(*a)`
                // Directly desugaring a splat node is a destructive operation, which can leave the "expr" in an invalid
                // state (because it would have a null desugared expr), which is incompatible with the "fallback" path
                // (desugaring it as a Whitequark tree in `PrismDesugar.cc").
                //
                // It's only safe to do if we can be sure all adjacent elements (in an the same Array literal,
                // or arguments to the same method call) can also directly desugared.
                //
                // It's really hard to know that ahead of time, so for now, just deepCopy the tree, instead of taking
                // it out of the splatted expressions's `NodeWithExpr`.
                auto childExpr = expr->peekDesugaredExpr().deepCopy();
                auto splatExpr = MK::Splat(location, move(childExpr));
                return make_node_with_expr<parser::Splat>(move(splatExpr), location, move(expr));
            }
        }
        case PM_STATEMENTS_NODE: { // A sequence of statements, such a in a `begin` block, `()`, etc.
            return translateStatements(down_cast<pm_statements_node>(node));
        }
        case PM_STRING_NODE: { // A string literal, e.g. `"foo"`
            auto strNode = down_cast<pm_string_node>(node);

            auto unescaped = &strNode->unescaped;
            auto content = ctx.state.enterNameUTF8(parser.extractString(unescaped));

            return make_node_with_expr<parser::String>(MK::String(location, content), location, content);
        }
        case PM_SUPER_NODE: { // A `super` call with explicit args, like `super()`, `super(a, b)`
            // If there's no arguments (except a literal block argument), then it's a `PM_FORWARDING_SUPER_NODE`.

            auto superNode = down_cast<pm_super_node>(node);

            auto blockArgumentNode = superNode->block;
            NodeVec returnValues;

            if (blockArgumentNode) { // Adjust the location to exclude the literal block argument.
                const uint8_t *start = superNode->base.location.start;
                const uint8_t *end;

                if (superNode->rparen_loc.end) { // Try to use the location of the `)`, if any
                    end = superNode->rparen_loc.end;
                } else { // Otherwise, use the end of the last argument
                    auto *argP = superNode->arguments;

                    constexpr auto msg =
                        "`PM_SUPER_NODE` must have arguments if it has no parens. If there's no args and no "
                        "parens, then you wouldn't even have a `PM_SUPER_NODE` to begin with, but a "
                        "`PM_FORWARDING_SUPER` instead)";
                    ENFORCE(argP, msg);

                    auto args = absl::MakeSpan(argP->arguments.nodes, argP->arguments.size);
                    ENFORCE(!args.empty(), msg);
                    end = args.back()->location.end;
                }

                location = translateLoc(start, end);
            }

            if (blockArgumentNode != nullptr && PM_NODE_TYPE_P(blockArgumentNode, PM_BLOCK_NODE)) {
                returnValues = translateArguments(superNode->arguments);
                auto superNode = make_unique<parser::Super>(location, move(returnValues));
                return translateCallWithBlock(blockArgumentNode, move(superNode));
            }

            returnValues = translateArguments(superNode->arguments, blockArgumentNode);
            return make_unique<parser::Super>(location, move(returnValues));
        }
        case PM_SYMBOL_NODE: { // A symbol literal, e.g. `:foo`, or `a:` in `{a: 1}`
            auto symNode = down_cast<pm_symbol_node>(node);

            auto [content, location] = translateSymbol(symNode);

            return make_node_with_expr<parser::Symbol>(MK::Symbol(location, content), location, content);
        }
        case PM_TRUE_NODE: { // The `true` keyword
            return make_node_with_expr<parser::True>(MK::True(location), location);
        }
        case PM_UNDEF_NODE: { // The `undef` keyword, like `undef :method_to_undef
            auto undefNode = down_cast<pm_undef_node>(node);

            auto names = translateMulti(undefNode->names);
            auto numPosArgs = names.size();

            if (!directlyDesugar || !hasExpr(names)) {
                return make_unique<parser::Undef>(location, move(names));
            }

            auto args = nodeVecToStore<ast::Send::ARGS_store>(names);
            auto expr = MK::Send(location, MK::Constant(location, core::Symbols::Kernel()), core::Names::undef(),
                                 location.copyWithZeroLength(), numPosArgs, std::move(args));
            // It wasn't a Send to begin with--there's no way this could result in a private
            // method call error.
            ast::cast_tree_nonnull<ast::Send>(expr).flags.isPrivateOk = true;
            return make_node_with_expr<parser::Undef>(std::move(expr), location, std::move(names));
        }
        case PM_UNLESS_NODE: { // An `unless` branch, either in a statement or modifier form.
            auto unlessNode = down_cast<pm_unless_node>(node);

            auto predicate = translate(unlessNode->predicate);
            // These are flipped relative to `PM_IF_NODE`
            auto ifFalse = translate(up_cast(unlessNode->statements));
            auto ifTrue = translate(up_cast(unlessNode->else_clause));

            return translateIfNode(location, move(predicate), move(ifTrue), move(ifFalse));
        }
        case PM_UNTIL_NODE: { // A `until` loop, like `until stop_condition; ...; end`
            auto untilNode = down_cast<pm_until_node>(node);

            auto predicate = translate(untilNode->predicate);
            auto statements = translate(up_cast(untilNode->statements));

            // When the until loop is placed after a `begin` block, like `begin; end until false`,
            bool beginModifier = PM_NODE_FLAG_P(untilNode, PM_LOOP_FLAGS_BEGIN_MODIFIER);

            if (!hasExpr(predicate, statements)) {
                if (beginModifier) {
                    return make_unique<parser::UntilPost>(location, move(predicate), move(statements));
                } else {
                    return make_unique<parser::Until>(location, move(predicate), move(statements));
                }
            } else {
                auto cond = predicate->takeDesugaredExpr();
                auto body = statements ? statements->takeDesugaredExpr() : MK::EmptyTree();
                if (beginModifier) {
                    auto breaker =
                        MK::If(location, std::move(cond), MK::Break(location, MK::EmptyTree()), MK::EmptyTree());
                    auto breakWithBody = MK::InsSeq1(location, std::move(body), std::move(breaker));
                    ast::ExpressionPtr expr = MK::While(location, MK::True(location), std::move(breakWithBody));
                    return make_node_with_expr<parser::UntilPost>(move(expr), location, move(predicate),
                                                                  move(statements));
                } else {
                    // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                    auto negatedCond =
                        MK::Send0(location, std::move(cond), core::Names::bang(), location.copyWithZeroLength());
                    auto expr = MK::While(location, std::move(negatedCond), std::move(body));
                    return make_node_with_expr<parser::Until>(move(expr), location, move(predicate), move(statements));
                }
            }
        }
        case PM_WHEN_NODE: { // A `when` clause, as part of a `case` statement
            auto whenNode = down_cast<pm_when_node>(node);

            auto sorbetConditions = translateMulti(whenNode->conditions);
            auto statements = translateStatements(whenNode->statements);

            return make_unique<parser::When>(location, move(sorbetConditions), move(statements));
        }
        case PM_WHILE_NODE: { // A `while` loop, like `while condition; ...; end`
            auto whileNode = down_cast<pm_while_node>(node);

            auto predicate = translate(whileNode->predicate);
            auto statements = translate(up_cast(whileNode->statements));

            // When the while loop is placed after a `begin` block, like `begin; end while false`,
            bool beginModifier = PM_NODE_FLAG_P(whileNode, PM_LOOP_FLAGS_BEGIN_MODIFIER);

            if (!hasExpr(predicate, statements)) {
                if (beginModifier) {
                    return make_unique<parser::WhilePost>(location, move(predicate), move(statements));
                } else {
                    return make_unique<parser::While>(location, move(predicate), move(statements));
                }
            } else {
                auto cond = predicate->takeDesugaredExpr();
                auto body = statements ? statements->takeDesugaredExpr() : MK::EmptyTree();
                if (beginModifier) {
                    // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                    auto negatedCond =
                        MK::Send0(location, std::move(cond), core::Names::bang(), location.copyWithZeroLength());
                    auto breaker =
                        MK::If(location, std::move(negatedCond), MK::Break(location, MK::EmptyTree()), MK::EmptyTree());
                    auto breakWithBody = MK::InsSeq1(location, std::move(body), std::move(breaker));
                    ast::ExpressionPtr expr = MK::While(location, MK::True(location), std::move(breakWithBody));
                    return make_node_with_expr<parser::WhilePost>(move(expr), location, move(predicate),
                                                                  move(statements));
                } else {
                    auto expr = MK::While(location, std::move(cond), std::move(body));
                    return make_node_with_expr<parser::While>(move(expr), location, move(predicate), move(statements));
                }
            }
        }
        case PM_X_STRING_NODE: { // A non-interpolated x-string, like `/usr/bin/env ls`
            auto strNode = down_cast<pm_x_string_node>(node);

            auto unescaped = &strNode->unescaped;
            auto source = parser.extractString(unescaped);
            auto content = ctx.state.enterNameUTF8(source);
            auto contentLoc = translateLoc(strNode->content_loc);

            // Create the backtick send call for the desugared expression
            auto sendBacktick = MK::Send1(location, MK::Self(location), core::Names::backtick(),
                                          location.copyWithZeroLength(), MK::String(contentLoc, content));

            auto nodes =
                NodeVec1(make_node_with_expr<parser::String>(MK::String(contentLoc, content), contentLoc, content));

            return make_node_with_expr<parser::XString>(move(sendBacktick), location, move(nodes));
        }
        case PM_YIELD_NODE: { // The `yield` keyword, like `yield`, `yield 1, 2, 3`
            auto yieldNode = down_cast<pm_yield_node>(node);

            auto yieldArgs = translateArguments(yieldNode->arguments);

            if (!directlyDesugar || !hasExpr(yieldArgs)) {
                return make_unique<parser::Yield>(location, move(yieldArgs));
            }

            ExpressionPtr recv;
            if (enclosingBlockParamName.exists()) {
                if (enclosingBlockParamName == core::Names::blkArg()) {
                    if (auto e =
                            ctx.beginIndexerError(enclosingMethodLoc, core::errors::Desugar::UnnamedBlockParameter)) {
                        e.setHeader("Method `{}` uses `{}` but does not mention a block parameter",
                                    enclosingMethodName.show(ctx), "yield");
                        e.addErrorLine(ctx.locAt(location), "Arising from use of `{}` in method body", "yield");
                    }
                }
                recv = MK::Local(location, enclosingBlockParamName);
            } else {
                recv = MK::RaiseUnimplemented(location);
            }

            auto args = nodeVecToStore<ast::Send::ARGS_store>(yieldArgs);
            auto expr = MK::Send(location, std::move(recv), core::Names::call(), location.copyWithZeroLength(),
                                 args.size(), std::move(args));
            return make_node_with_expr<parser::Yield>(std::move(expr), location, move(yieldArgs));
        }

        case PM_ALTERNATION_PATTERN_NODE: // A pattern like `1 | 2`
        case PM_ARRAY_PATTERN_NODE:       // An array pattern such as the `[head, *tail]` in the `a in [head, *tail]`
        case PM_CAPTURE_PATTERN_NODE:     // A variable capture such as the `=> i` in `in Integer => i`
        case PM_FIND_PATTERN_NODE:        // A find pattern such as the `[*, middle, *]` in the `a in [*, middle, *]`
        case PM_HASH_PATTERN_NODE:        // An hash pattern such as the `{ k: Integer }` in the `h in { k: Integer }`
        case PM_IN_NODE:                // An `in` pattern such as in a `case` statement, or as a standalone expression.
        case PM_PINNED_EXPRESSION_NODE: // A "pinned" expression, like `^(1 + 2)` in `in ^(1 + 2)`
        case PM_PINNED_VARIABLE_NODE:   // A "pinned" variable, like `^x` in `in ^x`
            unreachable(
                "These pattern-match related nodes are handled separately in `Translator::patternTranslate()`.");

        case PM_SCOPE_NODE: // An internal node type only created by the MRI's Ruby compiler, and not Prism itself.
            unreachable("Prism's parser never produces `PM_SCOPE_NODE` nodes.");

        case PM_MISSING_NODE: {
            ast::ExpressionPtr expr =
                MK::UnresolvedConstant(location, MK::EmptyTree(), core::Names::Constants::ErrorNode());
            return make_node_with_expr<parser::Const>(move(expr), location, nullptr,
                                                      core::Names::Constants::ErrorNode());
        }
    }
}

core::LocOffsets Translator::translateLoc(const uint8_t *start, const uint8_t *end) const {
    return parser.translateLocation(start, end);
}

core::LocOffsets Translator::translateLoc(pm_location_t loc) const {
    return parser.translateLocation(loc);
}

// Find the start location of a node, according to the legacy parser's logic.
// This is *usually* the same as the start of Prism's node's location,
// but there are some exceptions, which get handled here.
const uint8_t *startLoc(pm_node_t *anyNode) {
    switch (PM_NODE_TYPE(anyNode)) {
        case PM_MULTI_TARGET_NODE: {
            // TODO: Delete this case when https://github.com/sorbet/sorbet/issues/9630 is fixed
            auto *node = down_cast<pm_multi_target_node>(anyNode);
            return mlhsLocation(node).start;
        }
        case PM_MULTI_WRITE_NODE: {
            // TODO: Delete this case when https://github.com/sorbet/sorbet/issues/9630 is fixed
            auto *node = down_cast<pm_multi_write_node>(anyNode);
            return mlhsLocation(node).start;
        }
        default: {
            return anyNode->location.start;
        }
    }
}

// End counterpart of `startLoc()`. See its docs for details.
const uint8_t *endLoc(pm_node_t *anyNode) {
    switch (PM_NODE_TYPE(anyNode)) {
        case PM_MULTI_TARGET_NODE: {
            // TODO: Delete this case when https://github.com/sorbet/sorbet/issues/9630 is fixed
            auto *node = down_cast<pm_multi_target_node>(anyNode);
            return mlhsLocation(node).end;
        }
        case PM_MULTI_WRITE_NODE: {
            // TODO: Delete this case when https://github.com/sorbet/sorbet/issues/9630 is fixed
            auto *node = down_cast<pm_multi_write_node>(anyNode);
            return endLoc(node->value);
        }
        default: {
            return anyNode->location.end;
        }
    }
}

// Translates a Prism node list into a new `NodeVec` of legacy parser nodes.
parser::NodeVec Translator::translateMulti(pm_node_list nodeList) {
    auto prismNodes = absl::MakeSpan(nodeList.nodes, nodeList.size);

    parser::NodeVec result;

    // Pre-allocate the exactly capacity we're going to need, to prevent growth reallocations.
    result.reserve(prismNodes.size());

    translateMultiInto(result, prismNodes);

    return result;
}

// Translates the given Prism nodes, and appends them to the given `NodeVec` of Sorbet nodes.
void Translator::translateMultiInto(NodeVec &outSorbetNodes, absl::Span<pm_node_t *> prismNodes) {
    for (auto &prismNode : prismNodes)
        outSorbetNodes.emplace_back(translate(prismNode));
}

// Similar to `translate()`, but it's used for pattern-matching nodes.
//
// This is necessary because some Prism nodes get translated differently depending on whether they're part of "regular"
// syntax, or pattern-matching syntax.
//
// E.g. `PM_LOCAL_VARIABLE_TARGET_NODE` normally translates to `parser::LVarLhs`, but `parser::MatchVar` in the context
// of a pattern.
unique_ptr<parser::Node> Translator::patternTranslate(pm_node_t *node) {
    if (node == nullptr)
        return nullptr;

    auto location = translateLoc(node->location);

    switch (PM_NODE_TYPE(node)) {
        case PM_ALTERNATION_PATTERN_NODE: { // A pattern like `1 | 2`
            auto alternationPatternNode = down_cast<pm_alternation_pattern_node>(node);

            auto left = patternTranslate(alternationPatternNode->left);
            auto right = patternTranslate(alternationPatternNode->right);

            if (!directlyDesugar || !hasExpr(left, right)) {
                return make_unique<parser::MatchAlt>(location, move(left), move(right));
            }

            // Like array/hash patterns, MatchAlt is a structural pattern that doesn't have
            // a simple desugared expression - it's handled specially during pattern matching desugaring
            return make_node_with_expr<parser::MatchAlt>(MK::Nil(location), location, move(left), move(right));
        }
        case PM_ASSOC_NODE: { // A key-value pair in a Hash pattern, e.g. the `k: v` in `h in { k: v }
            auto assocNode = down_cast<pm_assoc_node>(node);

            // If the value is an implicit node, skip creating the pair, and return that value directly.
            if (PM_NODE_TYPE_P(assocNode->value, PM_IMPLICIT_NODE)) {
                auto implicitNode = down_cast<pm_implicit_node>(assocNode->value);

                // Special case: the legacy parser includes the colon's loc in a MatchVar if it's in a Hash pattern key
                //     nil in { "n1": }
                //              ^^^^^
                if (PM_NODE_TYPE_P(implicitNode->value, PM_LOCAL_VARIABLE_TARGET_NODE)) {
                    auto localVarTargetNode = down_cast<pm_local_variable_target_node>(implicitNode->value);
                    auto name = translateConstantName(localVarTargetNode->name);

                    // Use the location of the assoc node:
                    return make_unique<MatchVar>(location, name);
                }

                return patternTranslate(assocNode->value);
            }

            auto key = patternTranslate(assocNode->key);
            auto value = patternTranslate(assocNode->value);

            if (PM_NODE_TYPE_P(assocNode->value, PM_IMPLICIT_NODE)) {
                return value;
            }

            if (!directlyDesugar || !hasExpr(key, value)) {
                return make_unique<parser::Pair>(location, move(key), move(value));
            }

            // Pair is a structural component of hash patterns with no simple desugared expression
            return make_node_with_expr<parser::Pair>(MK::Nil(location), location, move(key), move(value));
        }
        case PM_ARRAY_PATTERN_NODE: { // An array pattern such as the `[head, *tail]` in the `a in [head, *tail]`
            auto arrayPatternNode = down_cast<pm_array_pattern_node>(node);

            auto prismPrefixNodes = absl::MakeSpan(arrayPatternNode->requireds.nodes, arrayPatternNode->requireds.size);
            auto prismRestNode = arrayPatternNode->rest;
            auto prismSuffixNodes = absl::MakeSpan(arrayPatternNode->posts.nodes, arrayPatternNode->posts.size);

            NodeVec sorbetElements{};
            sorbetElements.reserve(prismPrefixNodes.size() + (prismRestNode != nullptr ? 1 : 0) +
                                   prismSuffixNodes.size());

            patternTranslateMultiInto(sorbetElements, prismPrefixNodes);

            // Implicit rest nodes in array patterns don't need to be translated
            if (prismRestNode != nullptr && !PM_NODE_TYPE_P(prismRestNode, PM_IMPLICIT_REST_NODE)) {
                sorbetElements.emplace_back(patternTranslate(prismRestNode));
            }

            patternTranslateMultiInto(sorbetElements, prismSuffixNodes);

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

            unique_ptr<parser::Node> arrayPattern = nullptr;

            // When the pattern ends with an implicit rest node, we need to return an `ArrayPatternWithTail` instead
            if (prismRestNode != nullptr && PM_NODE_TYPE_P(prismRestNode, PM_IMPLICIT_REST_NODE)) {
                if (!directlyDesugar || !hasExpr(sorbetElements)) {
                    arrayPattern = make_unique<parser::ArrayPatternWithTail>(patternLoc, move(sorbetElements));
                } else {
                    // ArrayPatternWithTail is a structural pattern with no direct desugared expression
                    arrayPattern = make_node_with_expr<parser::ArrayPatternWithTail>(MK::Nil(patternLoc), patternLoc,
                                                                                     move(sorbetElements));
                }
            } else {
                if (!directlyDesugar || !hasExpr(sorbetElements)) {
                    arrayPattern = make_unique<parser::ArrayPattern>(patternLoc, move(sorbetElements));
                } else {
                    // ArrayPattern is a structural pattern with no direct desugared expression
                    arrayPattern = make_node_with_expr<parser::ArrayPattern>(MK::Nil(patternLoc), patternLoc,
                                                                             move(sorbetElements));
                }
            }

            if (auto *prismConstant = arrayPatternNode->constant) {
                // An array pattern can start with a constant that matches against a specific type,
                // (rather than any value whose `#deconstruct` results are matched by the pattern).
                // E.g. the `Point` in `in Point[1, 2]`
                auto sorbetConstant = translate(prismConstant);

                if (!directlyDesugar || !hasExpr(sorbetConstant, arrayPattern)) {
                    return make_unique<parser::ConstPattern>(location, move(sorbetConstant), move(arrayPattern));
                }

                // ConstPattern wrapping the array pattern - the desugared expression is Nil as it's structural
                auto constPatternExpr = MK::Nil(location);
                return make_node_with_expr<parser::ConstPattern>(move(constPatternExpr), location, move(sorbetConstant),
                                                                 move(arrayPattern));
            }

            return arrayPattern;
        }
        case PM_CAPTURE_PATTERN_NODE: { // A variable capture such as the `Integer => i` in `in Integer => i`
            auto capturePatternNode = down_cast<pm_capture_pattern_node>(node);

            auto pattern = patternTranslate(capturePatternNode->value);
            auto target = patternTranslate(up_cast(capturePatternNode->target));

            return make_unique<parser::MatchAs>(location, move(pattern), move(target));
        }
        case PM_FIND_PATTERN_NODE: { // A find pattern such as the `[*, middle, *]` in the `a in [*, middle, *]`
            auto findPatternNode = down_cast<pm_find_pattern_node>(node);

            auto prismLeadingSplat = findPatternNode->left;
            auto prismMiddleNodes = absl::MakeSpan(findPatternNode->requireds.nodes, findPatternNode->requireds.size);
            auto prismTrailingSplat = findPatternNode->right;

            NodeVec sorbetElements{};
            sorbetElements.reserve(1 + prismMiddleNodes.size() + (prismTrailingSplat != nullptr ? 1 : 0));

            if (prismLeadingSplat != nullptr) {
                auto prismSplatNode = prismLeadingSplat;
                auto expr = patternTranslate(prismSplatNode->expression);
                auto splatLoc = translateLoc(prismSplatNode->base.location);

                if (!directlyDesugar || !hasExpr(expr)) {
                    sorbetElements.emplace_back(make_unique<MatchRest>(splatLoc, move(expr)));
                } else {
                    sorbetElements.emplace_back(
                        make_node_with_expr<parser::MatchRest>(MK::Nil(splatLoc), splatLoc, move(expr)));
                }
            }

            patternTranslateMultiInto(sorbetElements, prismMiddleNodes);

            if (prismTrailingSplat != nullptr && PM_NODE_TYPE_P(prismTrailingSplat, PM_SPLAT_NODE)) {
                // TODO: handle PM_NODE_TYPE_P(prismTrailingSplat, PM_MISSING_NODE)
                auto prismSplatNode = down_cast<pm_splat_node>(prismTrailingSplat);
                auto expr = patternTranslate(prismSplatNode->expression);
                auto splatLoc = translateLoc(prismSplatNode->base.location);

                if (!directlyDesugar || !hasExpr(expr)) {
                    sorbetElements.emplace_back(make_unique<MatchRest>(splatLoc, move(expr)));
                } else {
                    sorbetElements.emplace_back(
                        make_node_with_expr<parser::MatchRest>(MK::Nil(splatLoc), splatLoc, move(expr)));
                }
            }

            if (!directlyDesugar || !hasExpr(sorbetElements)) {
                return make_unique<parser::FindPattern>(location, move(sorbetElements));
            }

            // FindPattern is a structural pattern with no simple desugared expression
            return make_node_with_expr<parser::FindPattern>(MK::Nil(location), location, move(sorbetElements));
        }
        case PM_HASH_PATTERN_NODE: { // An hash pattern such as the `{ k: Integer }` in the `h in { k: Integer }`
            auto hashPatternNode = down_cast<pm_hash_pattern_node>(node);

            auto prismElements = absl::MakeSpan(hashPatternNode->elements.nodes, hashPatternNode->elements.size);
            auto prismRestNode = hashPatternNode->rest;

            NodeVec sorbetElements{};
            sorbetElements.reserve(prismElements.size() + (prismRestNode != nullptr ? 1 : 0));

            patternTranslateMultiInto(sorbetElements, prismElements);
            if (prismRestNode != nullptr) {
                auto loc = translateLoc(prismRestNode->location);

                switch (PM_NODE_TYPE(prismRestNode)) {
                    case PM_ASSOC_SPLAT_NODE: {
                        auto assocSplatNode = down_cast<pm_assoc_splat_node>(prismRestNode);
                        auto value = patternTranslate(assocSplatNode->value);

                        if (!directlyDesugar || !hasExpr(value)) {
                            sorbetElements.emplace_back(make_unique<parser::MatchRest>(loc, move(value)));
                        } else {
                            sorbetElements.emplace_back(
                                make_node_with_expr<parser::MatchRest>(MK::Nil(loc), loc, move(value)));
                        }
                        break;
                    }
                    case PM_NO_KEYWORDS_PARAMETER_NODE: {
                        if (!directlyDesugar) {
                            sorbetElements.emplace_back(make_unique<parser::MatchNilPattern>(loc));
                        } else {
                            sorbetElements.emplace_back(
                                make_node_with_expr<parser::MatchNilPattern>(MK::Nil(loc), loc));
                        }
                        break;
                    }
                    default:
                        sorbetElements.emplace_back(patternTranslate(prismRestNode));
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

            unique_ptr<parser::Node> hashPattern = nullptr;

            if (!directlyDesugar || !hasExpr(sorbetElements)) {
                hashPattern = make_unique<parser::HashPattern>(patternLoc, move(sorbetElements));
            } else {
                // HashPattern is a structural pattern with no direct desugared expression
                hashPattern =
                    make_node_with_expr<parser::HashPattern>(MK::Nil(patternLoc), patternLoc, move(sorbetElements));
            }

            if (auto *prismConstant = hashPatternNode->constant) {
                // A hash pattern can start with a constant that matches against a specific type,
                // (rather than any value whose `#deconstruct_keys` results are matched by the pattern).
                // E.g. the `Point` in `in Point[x: Integer => 1, y: Integer => 2]`
                auto sorbetConstant = translate(prismConstant);

                if (!directlyDesugar || !hasExpr(sorbetConstant, hashPattern)) {
                    return make_unique<parser::ConstPattern>(location, move(sorbetConstant), move(hashPattern));
                }

                // ConstPattern wrapping the hash pattern - the desugared expression is Nil as it's structural
                auto constPatternExpr = MK::Nil(location);
                return make_node_with_expr<parser::ConstPattern>(move(constPatternExpr), location, move(sorbetConstant),
                                                                 move(hashPattern));
            }

            return hashPattern;
        }
        case PM_IMPLICIT_NODE: {
            auto implicitNode = down_cast<pm_implicit_node>(node);
            return patternTranslate(implicitNode->value);
        }
        case PM_IN_NODE: { // An `in` pattern such as in a `case` statement, or as a standalone expression.
            auto inNode = down_cast<pm_in_node>(node);

            auto prismPattern = inNode->pattern;
            unique_ptr<parser::Node> sorbetPattern;
            unique_ptr<parser::Node> sorbetGuard;
            auto statements = translateStatements(inNode->statements);

            if (prismPattern != nullptr &&
                (PM_NODE_TYPE_P(prismPattern, PM_IF_NODE) || PM_NODE_TYPE_P(prismPattern, PM_UNLESS_NODE))) {
                pm_statements_node *conditionalStatements = nullptr;

                if (PM_NODE_TYPE_P(prismPattern, PM_IF_NODE)) {
                    auto ifNode = down_cast<pm_if_node>(prismPattern);
                    conditionalStatements = ifNode->statements;
                    auto location = translateLoc(ifNode->if_keyword_loc.start, ifNode->base.location.end);
                    sorbetGuard = make_unique<parser::IfGuard>(location, translate(ifNode->predicate));
                } else { // PM_UNLESS_NODE
                    ENFORCE(PM_NODE_TYPE_P(prismPattern, PM_UNLESS_NODE));
                    auto unlessNode = down_cast<pm_unless_node>(prismPattern);
                    conditionalStatements = unlessNode->statements;
                    auto location = translateLoc(unlessNode->keyword_loc.start, unlessNode->base.location.end);
                    sorbetGuard = make_unique<parser::UnlessGuard>(location, translate(unlessNode->predicate));
                }

                ENFORCE(
                    conditionalStatements->body.size == 1,
                    "In pattern-matching's `in` clause, a conditional (if/unless) guard must have a single statement.");

                sorbetPattern = patternTranslate(conditionalStatements->body.nodes[0]);
            } else {
                sorbetPattern = patternTranslate(prismPattern);
            }

            if (!directlyDesugar || !hasExpr(sorbetPattern) || !hasExpr(statements)) {
                return make_unique<parser::InPattern>(location, move(sorbetPattern), move(sorbetGuard),
                                                      move(statements));
            }

            // A single `in` clause does not desugar into a standalone Ruby expression; it only
            // becomes meaningful when the enclosing `case` stitches together all clauses. Wrapping it
            // in a NodeWithExpr seeded with `EmptyTree` satisfies the API contract so that
            // `hasExpr(inNodes)` can succeed. The enclosing `case` later consumes the real
            // expressions from the pattern and body when it assembles the final AST.
            return make_node_with_expr<parser::InPattern>(MK::EmptyTree(), location, move(sorbetPattern),
                                                          move(sorbetGuard), move(statements));
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // A variable binding in a pattern, like the `head` in `[head, *tail]`
            auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
            auto name = translateConstantName(localVarTargetNode->name);

            if (!directlyDesugar) {
                return make_unique<MatchVar>(location, name);
            }

            // For a match variable, the desugared expression is a local variable reference
            // This represents what the variable will be bound to when the pattern matches
            auto expr = MK::Local(location, name);
            return make_node_with_expr<parser::MatchVar>(move(expr), location, name);
        }
        case PM_PINNED_EXPRESSION_NODE: { // A "pinned" expression, like `^(1 + 2)` in `in ^(1 + 2)`
            auto pinnedExprNode = down_cast<pm_pinned_expression_node>(node);

            auto expr = translate(pinnedExprNode->expression);

            // Sorbet's parser always wraps the pinned expression in a `Begin` node.
            auto statements = NodeVec1(move(expr));
            auto beginNodeLocation = translateLoc(pinnedExprNode->lparen_loc.start, pinnedExprNode->rparen_loc.end);
            auto beginNode =
                make_node_with_expr<parser::Begin>(MK::Nil(beginNodeLocation), beginNodeLocation, move(statements));

            if (!directlyDesugar || !hasExpr(beginNode)) {
                return make_unique<Pin>(location, move(beginNode));
            }

            // For pinned expressions, the desugared expression comes from the begin node
            auto pinExpr = beginNode->takeDesugaredExpr();
            return make_node_with_expr<parser::Pin>(move(pinExpr), location, move(beginNode));
        }
        case PM_PINNED_VARIABLE_NODE: { // A "pinned" variable, like `^x` in `in ^x`
            auto pinnedVarNode = down_cast<pm_pinned_variable_node>(node);

            auto variable = translate(pinnedVarNode->variable);

            if (!directlyDesugar || !hasExpr(variable)) {
                return make_unique<Pin>(location, move(variable));
            }

            // For pinned variables, the desugared expression is just the variable's expression
            auto expr = variable->takeDesugaredExpr();
            return make_node_with_expr<parser::Pin>(move(expr), location, move(variable));
        }
        case PM_SPLAT_NODE: { // A splat, like `*a` in an array pattern
            auto prismSplatNode = down_cast<pm_splat_node>(node);
            auto expr = patternTranslate(prismSplatNode->expression);

            if (!directlyDesugar || !hasExpr(expr)) {
                return make_unique<MatchRest>(location, move(expr));
            }

            // MatchRest is a structural pattern component with no simple desugared expression
            return make_node_with_expr<parser::MatchRest>(MK::Nil(location), location, move(expr));
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

            return make_node_with_expr<parser::Symbol>(MK::Symbol(location, content), location, content);
        }
        default: {
            return translate(node);
        }
    }
}

// Translates a Prism node list into a new `NodeVec` of legacy parser nodes.
// This is like `translateMulti()`, but calls `patternTranslateMultiInto()` instead of `translateMultiInto()`.
parser::NodeVec Translator::patternTranslateMulti(pm_node_list nodeList) {
    auto prismNodes = absl::MakeSpan(nodeList.nodes, nodeList.size);

    parser::NodeVec result;

    // Pre-allocate the exactly capacity we're going to need, to prevent growth reallocations.
    result.reserve(prismNodes.size());

    patternTranslateMultiInto(result, prismNodes);

    return result;
}

// Translates the given Prism pattern-matching nodes, and appends them to the given `NodeVec` of Sorbet nodes.
// This is like `translateMultiInto()`, but calls `patternTranslate()` instead of `translate()`.
void Translator::patternTranslateMultiInto(NodeVec &outSorbetNodes, absl::Span<pm_node_t *> prismNodes) {
    for (auto &prismNode : prismNodes) {
        unique_ptr<parser::Node> sorbetNode = patternTranslate(prismNode);
        outSorbetNodes.emplace_back(move(sorbetNode));
    }
}

pair<unique_ptr<parser::Params>, core::NameRef /* enclosingBlockParamName */>
Translator::translateParametersNode(pm_parameters_node *paramsNode, core::LocOffsets location) {
    auto requireds = absl::MakeSpan(paramsNode->requireds.nodes, paramsNode->requireds.size);
    auto optionals = absl::MakeSpan(paramsNode->optionals.nodes, paramsNode->optionals.size);
    auto keywords = absl::MakeSpan(paramsNode->keywords.nodes, paramsNode->keywords.size);
    auto posts = absl::MakeSpan(paramsNode->posts.nodes, paramsNode->posts.size);

    parser::NodeVec params;

    auto restSize = paramsNode->rest == nullptr ? 0 : 1;
    auto kwrestSize = paramsNode->keyword_rest == nullptr ? 0 : 1;
    auto blockSize = paramsNode->block == nullptr ? 0 : 1;

    params.reserve(requireds.size() + optionals.size() + restSize + posts.size() + keywords.size() + kwrestSize +
                   blockSize);

    for (auto &n : requireds) {
        if (PM_NODE_TYPE_P(n, PM_MULTI_TARGET_NODE)) {
            auto multiTargetNode = down_cast<pm_multi_target_node>(n);

            ENFORCE(multiTargetNode->lparen_loc.start);
            ENFORCE(multiTargetNode->lparen_loc.end);
            ENFORCE(multiTargetNode->rparen_loc.start);
            ENFORCE(multiTargetNode->rparen_loc.end);

            // The legacy parser doesn't usually include the parens in the location of a multi-target node,
            // *except* in a block's parameter list.
            auto mlhsLoc = translateLoc(multiTargetNode->lparen_loc.start, multiTargetNode->rparen_loc.end);

            auto multiLhsNode = translateMultiTargetLhs(multiTargetNode, mlhsLoc);

            params.emplace_back(move(multiLhsNode));
        } else {
            params.emplace_back(translate(n));
        }
    }

    translateMultiInto(params, optionals);

    if (paramsNode->rest != nullptr) {
        params.emplace_back(translate(paramsNode->rest));
    }

    for (auto &prismNode : posts) {
        // Valid Ruby can only have `**nil` once in the parameter list, which is modelled with a
        // `NoKeywordsParameterNode` in the `keyword_rest` field.
        // If invalid code tries to use more than one `**nil` (like `def foo(**nil, **nil)`),
        // Prism will report an error, but still place the excess `**nil` nodes in `posts` list (never the others like
        // `requireds` or `optionals`), which we need to skip here.
        if (!PM_NODE_TYPE_P(prismNode, PM_NO_KEYWORDS_PARAMETER_NODE)) {
            params.emplace_back(translate(prismNode));
        }
    }

    translateMultiInto(params, keywords);

    bool hasForwardingParameter = false;
    if (auto *prismKwRestNode = paramsNode->keyword_rest) {
        switch (PM_NODE_TYPE(prismKwRestNode)) {
            case PM_KEYWORD_REST_PARAMETER_NODE: // `def foo(**kwargs)`
                params.emplace_back(translate(prismKwRestNode));
                break;
            case PM_FORWARDING_PARAMETER_NODE: { // `def foo(...)`
                hasForwardingParameter = true;
                params.emplace_back(translate(prismKwRestNode));
                break;
            }
            case PM_NO_KEYWORDS_PARAMETER_NODE: { // `def foo(**nil)`
                params.emplace_back(make_unique<parser::Kwnilarg>(translateLoc(prismKwRestNode->location)));
                break;
            }
            default:
                unreachable("Unexpected keyword_rest node type in Hash pattern.");
        }
    }

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
            enclosingBlockParamName = nextUniqueParserName(core::Names::ampersand());
        }

        auto blockParamExpr = MK::BlockParam(blockParamLoc, MK::Local(blockParamLoc, enclosingBlockParamName));
        auto blockParamNode =
            make_node_with_expr<parser::BlockParam>(move(blockParamExpr), blockParamLoc, enclosingBlockParamName);

        params.emplace_back(move(blockParamNode));
    } else {
        // Desugaring a method def like `def foo(a, b)` should behave like `def foo(a, b, &<blk>)`,
        // so we set a synthetic name here for `yield` to use.
        enclosingBlockParamName = core::Names::blkArg();
    }

    return {make_unique<parser::Params>(location, move(params)), enclosingBlockParamName};
}

tuple<ast::MethodDef::PARAMS_store, ast::InsSeq::STATS_store, bool /* didDesugarParams */>
Translator::desugarParametersNode(NodeVec &params, bool attemptToDesugarParams) {
    if (!attemptToDesugarParams) {
        return make_tuple(ast::MethodDef::PARAMS_store{}, ast::InsSeq::STATS_store{}, false);
    }

    auto supportedParams = absl::c_all_of(params, [](auto &param) {
        return param->hasDesugaredExpr() ||
               // These other block types don't have their own dedicated desugared
               // representation, so they won't be directly translated.
               // Instead, they have special desugar logic below.
               parser::NodeWithExpr::isa_node<parser::Kwnilarg>(param.get()) ||         // `def f(**nil)`
               parser::NodeWithExpr::isa_node<parser::ForwardArg>(param.get()) ||       // `def f(...)`
               parser::NodeWithExpr::isa_node<parser::ForwardedRestArg>(param.get()) || // a splat like `def foo(*)`
               parser::NodeWithExpr::isa_node<parser::Splat>(param.get());              // a splat like `def foo(*a)`
    });

    if (!supportedParams) {
        return make_tuple(ast::MethodDef::PARAMS_store{}, ast::InsSeq::STATS_store{}, false);
    }

    ast::MethodDef::PARAMS_store paramsStore;
    ast::InsSeq::STATS_store statsStore;

    for (auto &param : params) {
        auto loc = param->loc;

        if (parser::NodeWithExpr::isa_node<parser::Mlhs>(param.get())) { // `def f((a, b))`
            unreachable("Support for Mlhs is not implemented yet!");
        } else if (parser::NodeWithExpr::isa_node<parser::Kwnilarg>(param.get())) { // `def foo(**nil)`
            // TODO: implement logic for `**nil` args
        } else if (parser::NodeWithExpr::isa_node<parser::ForwardArg>(param.get())) { // `def foo(...)`
            // Desugar `def foo(m, n, ...)` into:
            // `def foo(m, n, *<fwd-args>, **<fwd-kwargs>, &<fwd-block>)`

            // add `*<fwd-args>`
            paramsStore.emplace_back(MK::RestParam(loc, MK::Local(loc, core::Names::fwdArgs())));

            // add `**<fwd-kwargs>`
            paramsStore.emplace_back(MK::RestParam(loc, MK::KeywordArg(loc, core::Names::fwdKwargs())));

            // add `&<fwd-block>`
            paramsStore.emplace_back(MK::BlockParam(loc, MK::Local(loc, core::Names::fwdBlock())));
        } else {
            paramsStore.emplace_back(param->takeDesugaredExpr());
        }
    }

    return make_tuple(move(paramsStore), move(statsStore), true);
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

// Translate the given numbered parameters into a `NodeVec` of legacy parser nodes.
// If a paramsStore pointer is provided, we'll also directly desugar params into that store.
NodeVec Translator::translateNumberedParametersNode(pm_numbered_parameters_node *numberedParamsNode,
                                                    pm_statements_node_t *statements,
                                                    ast::MethodDef::PARAMS_store *paramsStore) {
    auto location = translateLoc(numberedParamsNode->base.location);

    auto paramCount = numberedParamsNode->maximum;

    ENFORCE(1 <= paramCount, "A `pm_numbered_parameters_node_t` node should have at least one parameter");
    ENFORCE(paramCount <= 9, "Ruby only supports 9 numbered parameters (`_9` but no `_10`).");

    auto numberedParamsUsageLocs = findNumberedParamsUsageLocs(location, statements, paramCount);
    ENFORCE(paramCount <= numberedParamsUsageLocs.size());

    NodeVec params;
    params.reserve(paramCount);

    for (auto i = 1; i <= paramCount; i++) {
        // Numbered parameters are implicit, so they don't have a real location in the body.
        // However, we need somewhere for the error messages to point to, so we use the
        // location of the first *usage* of this numbered parameter (or none if it was never used).
        auto usageLoc = numberedParamsUsageLocs[i - 1];
        auto name = ctx.state.enterNameUTF8("_" + to_string(i));

        if (usageLoc.exists()) {
            // The legacy parse tree only includes parameters that were used in the body.
            params.emplace_back(make_node_with_expr<parser::LVar>(MK::Local(usageLoc, name), usageLoc, name));
        }

        if (paramsStore) {
            paramsStore->emplace_back(MK::Local(usageLoc, name));
        }
    }

    // The legacy parse tree stores params in the order they were encountered in the body.
    fast_sort(params, [](const auto &a, const auto &b) { return a->loc.beginLoc < b->loc.beginLoc; });

    return params;
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
    ENFORCE(directlyDesugar, "desugarSymbolProc should only be called when direct desugaring is enabled");

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

// The legacy Sorbet parser doesn't have a counterpart to PM_ARGUMENTS_NODE to wrap the array
// of argument nodes. It just uses a NodeVec directly, which is what this function produces.
NodeVec Translator::translateArguments(pm_arguments_node *argsNode, pm_node *blockArgumentNode) {
    NodeVec results;

    absl::Span<pm_node *> prismArgs;

    if (argsNode != nullptr) {
        prismArgs = absl::MakeSpan(argsNode->arguments.nodes, argsNode->arguments.size);
    }

    results.reserve(prismArgs.size() + (blockArgumentNode == nullptr ? 0 : 1));

    translateMultiInto(results, prismArgs);
    if (blockArgumentNode != nullptr) {
        results.emplace_back(translate(blockArgumentNode));
    }

    return results;
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

// Translates the given Prism elements into a `NodeVec` of legacy parser nodes.
// The elements are are usually key/value pairs, but can also be Hash splats (`**`).
//
// This method is used by:
//   * PM_HASH_NODE (Hash literals)
//   * PM_KEYWORD_HASH_NODE (keyword arguments to a method call)
//
// @param elements The Prism key/value pairs to be translated
parser::NodeVec Translator::translateKeyValuePairs(pm_node_list_t elements) {
    auto prismElements = absl::MakeSpan(elements.nodes, elements.size);

    parser::NodeVec sorbetElements{};
    sorbetElements.reserve(prismElements.size());

    for (auto &element : prismElements) {
        if (PM_NODE_TYPE_P(element, PM_ASSOC_SPLAT_NODE)) {
            auto prismSplatNode = down_cast<pm_assoc_splat_node>(element);
            auto splatLoc = translateLoc(prismSplatNode->base.location);
            auto value = translate(prismSplatNode->value);

            unique_ptr<parser::Node> sorbetSplatNode;
            if (value == nullptr) { // An anonymous splat like `f(**)`
                sorbetSplatNode = make_unique<parser::ForwardedKwrestArg>(splatLoc);
            } else { // Splatting an expression like `f(**h)`
                sorbetSplatNode = make_unique<parser::Kwsplat>(splatLoc, move(value));
            }

            sorbetElements.emplace_back(move(sorbetSplatNode));
        } else {
            ENFORCE(PM_NODE_TYPE_P(element, PM_ASSOC_NODE))
            auto pair = down_cast<pm_assoc_node>(element);

            unique_ptr<parser::Node> sorbetKVPair;
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

                    auto key = make_node_with_expr<parser::Symbol>(MK::Symbol(symbolLoc, symbolContent), symbolLoc,
                                                                   symbolContent);
                    auto value = translate(pair->value);
                    sorbetKVPair =
                        make_unique<parser::Pair>(translateLoc(pair->base.location), move(key), translate(pair->value));

                } else {
                    sorbetKVPair = translate(element);
                }
            } else {
                sorbetKVPair = translate(element);
            }

            sorbetElements.emplace_back(move(sorbetKVPair));
        }
    }

    return sorbetElements;
}

ast::ExpressionPtr Translator::desugarHash(core::LocOffsets loc, NodeVec &kvPairs) {
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

        auto *pair = parser::NodeWithExpr::cast_node<parser::Pair>(pairAsExpression.get());
        if (pair != nullptr) {
            auto key = pair->key->takeDesugaredExpr();
            hashKeyDupes.check(key);
            mergeValues.emplace_back(move(key));

            auto value = pair->value->takeDesugaredExpr();
            mergeValues.emplace_back(move(value));

            havePairsToMerge = true;
            continue;
        }

        auto *splat = parser::NodeWithExpr::cast_node<parser::Kwsplat>(pairAsExpression.get());

        ast::ExpressionPtr expr;
        if (splat != nullptr) {
            expr = splat->expr->takeDesugaredExpr();
        } else {
            auto *fwdKwrestArg = parser::NodeWithExpr::cast_node<parser::ForwardedKwrestArg>(pairAsExpression.get());
            ENFORCE(fwdKwrestArg != nullptr, "kwsplat and fwdkwrestarg cast failed");

            auto fwdKwargs = MK::Local(loc, core::Names::fwdKwargs());
            expr = MK::Unsafe(loc, move(fwdKwargs));
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

// Prism models a call with an explicit block argument as a `pm_call_node` that contains a `pm_block_node`.
// Sorbet's legacy parser models this the other way around, as a parent `Block` with a child `Send`.
// Lambda literals also have a similar reverse structure between the 2 parsers.
//
// This function translates between the two, creating a `Block`node for the given `pm_block_node *`
// or `pm_lambda_node *`, and wrapping it around the given `Send` node.
unique_ptr<parser::Node> Translator::translateCallWithBlock(pm_node_t *prismBlockOrLambdaNode,
                                                            unique_ptr<parser::Node> sendNode) {
    pm_node_t *prismParametersNode;
    pm_node_t *prismBodyNode;
    auto blockLoc = translateLoc(prismBlockOrLambdaNode->location);
    if (PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_BLOCK_NODE)) {
        auto prismBlockNode = down_cast<pm_block_node>(prismBlockOrLambdaNode);
        prismParametersNode = prismBlockNode->parameters;
        prismBodyNode = prismBlockNode->body;
    } else {
        ENFORCE(PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_LAMBDA_NODE))
        auto prismLambdaNode = down_cast<pm_lambda_node>(prismBlockOrLambdaNode);
        prismParametersNode = prismLambdaNode->parameters;
        prismBodyNode = prismLambdaNode->body;
    }

    unique_ptr<parser::Node> parametersNode;
    if (prismParametersNode != nullptr) {
        if (PM_NODE_TYPE_P(prismParametersNode, PM_NUMBERED_PARAMETERS_NODE)) {
            core::LocOffsets numParamsLoc;
            if (PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_BLOCK_NODE)) {
                auto prismBlockNode = down_cast<pm_block_node>(prismBlockOrLambdaNode);

                // Use a 0-length loc just after the `do` or `{` token, as if you had written:
                //     do|_1, _2| ... end`
                //       ^
                //     {|_1, _2| ... }`
                //      ^
                numParamsLoc = translateLoc(prismBlockNode->opening_loc.end, prismBlockNode->opening_loc.end);
            } else {
                ENFORCE(PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_LAMBDA_NODE));
                auto prismLambdaNode = down_cast<pm_lambda_node>(prismBlockOrLambdaNode);

                // Use a 0-length loc just after the `->` token, as if you had written:
                //     ->(_1, _2) { ... }
                //       ^
                numParamsLoc = translateLoc(prismLambdaNode->operator_loc.end, prismLambdaNode->operator_loc.end);
            }

            auto numberedParamsNode = down_cast<pm_numbered_parameters_node>(prismParametersNode);

            auto params = translateNumberedParametersNode(numberedParamsNode,
                                                          down_cast<pm_statements_node>(prismBodyNode), nullptr);
            parametersNode = make_unique<parser::NumParams>(numParamsLoc, move(params));
        } else {
            parametersNode = translate(prismParametersNode);
        }
    }

    auto body = this->enterBlockContext().translate(prismBodyNode);

    // Modify send node's endLoc to be position before first space
    // This fixes location for cases like:
    //   Module.new do
    //     #: (Integer) -> void
    //     def bar(x); end
    //   end
    // Where we want the send node to only cover "Module.new", not the entire block.
    // This mirrors how WQ stores send location and is needed for RBS rewriting.
    if (sendNode->loc.exists()) {
        auto source = ctx.file.data(ctx).source();
        auto beginPos = sendNode->loc.beginPos();
        auto endPos = sendNode->loc.endPos();

        // Find block keyword (do or {) within the send node bounds
        auto doPos = source.find(" do", beginPos);
        auto bracePos = source.find("{", beginPos);

        auto blockPos = std::string_view::npos;
        if (doPos != std::string_view::npos && doPos < endPos) {
            blockPos = doPos;
        } else if (bracePos != std::string_view::npos && bracePos < endPos) {
            blockPos = bracePos;
        }

        if (blockPos != std::string_view::npos) {
            sendNode->loc = core::LocOffsets{beginPos, static_cast<uint32_t>(blockPos)};
        }
    }

    return make_unique<parser::Block>(blockLoc, move(sendNode), move(parametersNode), move(body));
}

// Prism represents a `begin ... rescue ... end` construct using a `pm_begin_node` that may contain:
//   - `statements`: the code before the `rescue` clauses (the main body).
//   - `rescue_clause`: a `pm_rescue_node` representing the first `rescue` clause.
//   - `else_clause`: an optional `pm_else_node` representing the `else` clause.
//
// Each `pm_rescue_node` represents a single `rescue` clause and is linked to subsequent `rescue` clauses via its
// `subsequent` pointer. Each `pm_rescue_node` contains:
//   - `exceptions`: the exceptions to rescue (e.g., `RuntimeError`).
//   - `reference`: the exception variable (e.g., `=> e`).
//   - `statements`: the body of the rescue clause.
//
// In contrast, Sorbet's legacy parser represents the same construct using a `Rescue` node that contains:
//   - `body`: the code before the `rescue` clauses (the main body).
//   - `rescue`: a list of `Resbody` nodes, each representing a `rescue` clause.
//   - `else_`: an optional node representing the `else` clause.
//
// This function and the PM_BEGIN_NODE case translate between the two representations by processing the `pm_rescue_node`
// (and its linked `subsequent` nodes) and assembling the corresponding `Rescue` and `Resbody` nodes in Sorbet's AST.
unique_ptr<parser::Node> Translator::translateRescue(pm_begin_node *parentBeginNode) {
    auto *prismRescueNode = parentBeginNode->rescue_clause;
    ENFORCE(prismRescueNode, "translateRescue() should only be called if there's a `rescue` clause.")

    NodeVec rescueBodies;
    bool allRescueBodiesHaveExpr = true;

    // Each `rescue` clause generates a `Resbody` node, which is a child of the `Rescue` node.
    for (pm_rescue_node *currentRescueNode = prismRescueNode; currentRescueNode != nullptr;
         currentRescueNode = currentRescueNode->subsequent) {
        // Translate the exception variable (e.g. the `=> e` in `rescue => e`)
        auto var = translate(currentRescueNode->reference);

        // Translate the body of the rescue clause
        auto rescueBody = translateStatements(currentRescueNode->statements);

        // Translate the exceptions being rescued (e.g., `RuntimeError` in `rescue RuntimeError`)
        auto exceptions = translateMulti(currentRescueNode->exceptions);

        auto exceptionsNodes = absl::MakeSpan(currentRescueNode->exceptions.nodes, currentRescueNode->exceptions.size);
        unique_ptr<parser::Node> exceptionsArray;
        if (!exceptionsNodes.empty()) {
            auto arrayLoc = translateLoc(exceptionsNodes.front()->location.start, exceptionsNodes.back()->location.end);

            if (!directlyDesugar || !hasExpr(exceptions)) {
                exceptionsArray = make_unique<parser::Array>(arrayLoc, move(exceptions));
            } else {
                // Check if there are any splats in the exceptions
                bool hasSplat =
                    absl::c_any_of(exceptionsNodes, [](auto *ex) { return PM_NODE_TYPE_P(ex, PM_SPLAT_NODE); });

                // Build ast::Array expression from exceptions
                auto exceptionStore = nodeVecToStore<ast::Array::ENTRY_store>(exceptions);

                ast::ExpressionPtr arrayExpr;
                if (hasSplat) {
                    // Use desugarArray to properly handle splats with concat() calls
                    arrayExpr = desugarArray(arrayLoc, exceptionsNodes, move(exceptionStore));
                } else {
                    // Simple case: just create an array without desugaring splats
                    arrayExpr = ast::make_expression<ast::Array>(arrayLoc, move(exceptionStore));
                }

                exceptionsArray = make_node_with_expr<parser::Array>(move(arrayExpr), arrayLoc, move(exceptions));
            }
        }

        auto resbodyLoc = translateLoc(currentRescueNode->base.location);
        auto rescueKeywordLoc = translateLoc(currentRescueNode->keyword_loc);

        // If there's a subsequent rescue clause, we want the previous resbody to end at the end of the line
        // before the subsequent rescue starts, instead of extending all the way to the subsequent rescue.
        //
        // For example, in this code:
        //   begin
        //   rescue => e
        //   rescue => e
        //     e #: as String
        //   end
        //
        // In Prism, the first `rescue` clause extends all the way to `end`, which would consume
        // the assertion comment. In Whitequark (WQ), the first `rescue` ends at the end of its line.
        // We need proper location for RBS rewriting.
        if (currentRescueNode->subsequent != nullptr) {
            auto subsequentLoc = translateLoc(currentRescueNode->subsequent->base.location);

            // We want to end just before the subsequent rescue begins
            // So we use the position right before the subsequent rescue starts
            auto endPos = subsequentLoc.beginPos();

            // Move back to find the end of the previous line (before any whitespace)
            const auto &source = ctx.file.data(ctx).source();
            while (endPos > resbodyLoc.beginPos() && isspace(source[endPos - 1])) {
                endPos--;
            }

            resbodyLoc = core::LocOffsets{resbodyLoc.beginPos(), endPos};
        }

        if (!directlyDesugar || !hasExpr(var, rescueBody, exceptionsArray)) {
            auto body = make_unique<parser::Resbody>(resbodyLoc, move(exceptionsArray), move(var), move(rescueBody));
            allRescueBodiesHaveExpr = false;
            rescueBodies.emplace_back(move(body));
        } else {
            // Build ast::RescueCase expression
            ast::RescueCase::EXCEPTION_store astExceptions;
            if (exceptionsArray != nullptr) {
                auto exceptionsExpr = exceptionsArray->takeDesugaredExpr();
                if (auto exceptionsArrayExpr = ast::cast_tree<ast::Array>(exceptionsExpr)) {
                    astExceptions.insert(astExceptions.end(), make_move_iterator(exceptionsArrayExpr->elems.begin()),
                                         make_move_iterator(exceptionsArrayExpr->elems.end()));
                } else if (!ast::isa_tree<ast::EmptyTree>(exceptionsExpr)) {
                    astExceptions.emplace_back(move(exceptionsExpr));
                }
            }

            ast::ExpressionPtr varExpr;
            ast::ExpressionPtr rescueBodyExpr;

            // Check what kind of variable we have
            bool isReference = var != nullptr && ast::isa_reference(var->peekDesugaredExpr());
            bool isLocal = var != nullptr && ast::isa_tree<ast::Local>(var->peekDesugaredExpr());

            if (isReference && !isLocal) {
                auto &expr = var->peekDesugaredExpr();
                if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(expr)) {
                    isLocal = ident->kind == ast::UnresolvedIdent::Kind::Local;
                }
            }

            if (isLocal) {
                // Regular local variable
                varExpr = var->takeDesugaredExpr();

                if (rescueBody != nullptr) {
                    rescueBodyExpr = rescueBody->takeDesugaredExpr();
                } else {
                    rescueBodyExpr = ast::MK::EmptyTree();
                }
            } else if (isReference) {
                // Non-local reference (lvalue exception variables like @ex, @@ex, $ex)
                // Create a temp variable and wrap the body
                auto rescueTemp = nextUniqueDesugarName(core::Names::rescueTemp());
                auto varLoc = var->loc;
                varExpr = ast::MK::Local(varLoc, rescueTemp);

                // Create InsSeq: { @ex = <rescueTemp>; <rescue body> }
                auto lhsExpr = var->takeDesugaredExpr();
                auto assignExpr = ast::MK::Assign(varLoc, move(lhsExpr), ast::MK::Local(varLoc, rescueTemp));

                ast::InsSeq::STATS_store stats;
                stats.emplace_back(move(assignExpr));

                auto bodyExpr = rescueBody != nullptr ? rescueBody->takeDesugaredExpr() : ast::MK::EmptyTree();
                rescueBodyExpr = ast::MK::InsSeq(varLoc, move(stats), move(bodyExpr));
            } else {
                // For bare rescue clauses with no variable, create a <rescueTemp> variable
                // Legacy parser uses zero-length location only when there are no exceptions AND no body,
                // otherwise uses full keyword location
                auto rescueTemp = nextUniqueDesugarName(core::Names::rescueTemp());
                auto syntheticVarLoc = (exceptionsArray == nullptr && rescueBody == nullptr)
                                           ? rescueKeywordLoc.copyWithZeroLength()
                                           : rescueKeywordLoc;
                varExpr = ast::MK::Local(syntheticVarLoc, rescueTemp);

                rescueBodyExpr = rescueBody != nullptr ? rescueBody->takeDesugaredExpr() : ast::MK::EmptyTree();
            }

            auto rescueCaseExpr = ast::make_expression<ast::RescueCase>(resbodyLoc, move(astExceptions), move(varExpr),
                                                                        move(rescueBodyExpr));

            auto body = make_node_with_expr<parser::Resbody>(move(rescueCaseExpr), resbodyLoc, move(exceptionsArray),
                                                             move(var), move(rescueBody));
            rescueBodies.emplace_back(move(body));
        }
    }

    auto bodyNode = translateStatements(parentBeginNode->statements);
    auto elseNode = translate(up_cast(parentBeginNode->else_clause));

    // Find the last rescue clause by traversing the linked list
    pm_rescue_node *lastRescueNode = prismRescueNode;
    while (lastRescueNode->subsequent != nullptr) {
        lastRescueNode = lastRescueNode->subsequent;
    }

    const uint8_t *rescueStart;
    // Determine the start location, prioritize: begin statements > else statements > rescue keyword
    if (auto *beginStmts = parentBeginNode->statements) {
        // If there are begin statements, start there
        rescueStart = beginStmts->base.location.start;
    } else if (auto *prismElseNode = parentBeginNode->else_clause) {
        if (auto *elseStmts = prismElseNode->statements) {
            // No begin statements, but there are else statements - start at else statements
            rescueStart = elseStmts->base.location.start;
        } else {
            // No begin statements and no else statements - start at rescue keyword
            rescueStart = prismRescueNode->keyword_loc.start;
        }
    } else {
        // No begin statements and no else clause - start at rescue keyword
        rescueStart = prismRescueNode->keyword_loc.start;
    }

    const uint8_t *rescueEnd;
    // Determine the end location, prioritize: rescue keyword < rescue statements < else statements
    if (auto *prismElseNode = parentBeginNode->else_clause) {
        if (auto *elseStmts = prismElseNode->statements) {
            // If there are else statements, end at their end
            rescueEnd = elseStmts->base.location.end;
        } else if (auto *rescueStmts = lastRescueNode->statements) {
            // No else statements but there are rescue statements
            rescueEnd = rescueStmts->base.location.end;
        } else {
            // No else statements and no rescue statements - use rescue keyword end
            rescueEnd = lastRescueNode->base.location.end;
        }
    } else if (auto *rescueStmts = lastRescueNode->statements) {
        // No else clause but there are rescue statements
        rescueEnd = rescueStmts->base.location.end;
    } else {
        // No else clause and no rescue statements - use rescue keyword end
        rescueEnd = lastRescueNode->base.location.end;
    }

    core::LocOffsets rescueLoc = translateLoc(rescueStart, rescueEnd);

    // Check if all nodes have expressions
    bool hasExpressions = hasExpr(bodyNode) && allRescueBodiesHaveExpr;

    // The `Rescue` node combines the main body, the rescue clauses, and the else clause.
    if (!directlyDesugar || !hasExpressions) {
        return make_unique<parser::Rescue>(rescueLoc, move(bodyNode), move(rescueBodies), move(elseNode));
    }

    // Build the ast::Rescue expression
    ast::ExpressionPtr bodyExpr;
    if (bodyNode != nullptr) {
        bodyExpr = bodyNode->takeDesugaredExpr();
    } else {
        bodyExpr = ast::MK::EmptyTree();
    }

    // Extract RescueCase expressions from each Resbody node
    ast::Rescue::RESCUE_CASE_store rescueCases;
    rescueCases.reserve(rescueBodies.size());

    for (auto &resbody : rescueBodies) {
        // Each Resbody should already have a RescueCase expression from make_node_with_expr
        auto rescueCaseExpr = resbody->takeDesugaredExpr();
        ENFORCE(ast::isa_tree<ast::RescueCase>(rescueCaseExpr), "resbody should contain a RescueCase expression");
        rescueCases.emplace_back(move(rescueCaseExpr));
    }

    // Extract the else expression
    ast::ExpressionPtr elseExpr;
    elseExpr = (elseNode != nullptr) ? elseNode->takeDesugaredExpr() : ast::MK::EmptyTree();

    // Build the ast::Rescue expression (ensure is EmptyTree since this is translateRescue, not translateEnsure)
    auto rescueExpr = ast::make_expression<ast::Rescue>(rescueLoc, move(bodyExpr), move(rescueCases), move(elseExpr),
                                                        ast::MK::EmptyTree());

    return make_node_with_expr<parser::Rescue>(move(rescueExpr), rescueLoc, move(bodyNode), move(rescueBodies),
                                               move(elseNode));
}

NodeVec Translator::translateEnsure(pm_begin_node *beginNode) {
    NodeVec statements;

    unique_ptr<parser::Node> translatedRescue;
    if (beginNode->rescue_clause != nullptr) {
        translatedRescue = translateRescue(beginNode);
    }

    if (auto *ensureNode = beginNode->ensure_clause) {
        // Handle `begin ... ensure ... end`
        // When both ensure and rescue are present, Sorbet's legacy parser puts the Rescue node inside the
        // Ensure node.
        auto bodyNode = translateStatements(beginNode->statements);
        auto ensureBody = translateStatements(ensureNode->statements);

        absl::Span<pm_node_t *> prismStatements;
        if (beginNode->statements) {
            prismStatements = absl::MakeSpan(beginNode->statements->body.nodes, beginNode->statements->body.size);
        }

        // Had to widen the type from `parser::Ensure` to `parser::Node` to handle `make_node_with_expr` correctly.
        // TODO: narrow the type back after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
        unique_ptr<parser::Node> translatedEnsure;
        if (translatedRescue != nullptr) {
            // When we have a rescue clause, the Ensure node should span from either:
            // - the begin statements start (if present), or
            // - the rescue keyword (if no begin statements)
            // to the end of the body (rescue/else clause or ensure statements, whichever comes last)
            const uint8_t *start = prismStatements.empty() ? beginNode->rescue_clause->keyword_loc.start
                                                           : beginNode->statements->base.location.start;

            const uint8_t *end;

            // If there are ensure statements, always extend to include them
            if (ensureNode->statements) {
                end = ensureNode->statements->base.location.end;
            } else {
                // No ensure statements, so find the end of the rescue clause (including else if present)
                pm_rescue_node *lastRescueNode = beginNode->rescue_clause;
                while (lastRescueNode->subsequent != nullptr) {
                    lastRescueNode = lastRescueNode->subsequent;
                }

                if (auto *prismElseNode = beginNode->else_clause) {
                    if (auto *elseStmts = prismElseNode->statements) {
                        end = elseStmts->base.location.end;
                    } else {
                        end = prismElseNode->base.location.end;
                    }
                } else if (auto *rescueStmts = lastRescueNode->statements) {
                    end = rescueStmts->base.location.end;
                } else {
                    // When the last rescue clause has no statements, use the end of the rescue clause itself
                    end = lastRescueNode->base.location.end;
                }
            }

            auto loc = translateLoc(start, end);

            if (!directlyDesugar || !hasExpr(translatedRescue, ensureBody)) {
                translatedEnsure = make_unique<parser::Ensure>(loc, move(translatedRescue), move(ensureBody));
            } else {
                // Build ast::Rescue expression with ensure field set
                // When we have both rescue and ensure, the translatedRescue is already an ast::Rescue,
                // so we just need to set its ensure field
                auto bodyExpr = translatedRescue->takeDesugaredExpr();
                auto rescue = ast::cast_tree<ast::Rescue>(bodyExpr);
                ENFORCE(rescue != nullptr, "translatedRescue should be a Rescue node");

                rescue->ensure = ensureBody != nullptr ? ensureBody->takeDesugaredExpr() : ast::MK::EmptyTree();

                translatedEnsure =
                    make_node_with_expr<parser::Ensure>(move(bodyExpr), loc, move(translatedRescue), move(ensureBody));
            }
        } else {
            // When there's no rescue clause, the Ensure node location depends on whether there are begin statements:
            // - If there are begin statements: span from start of begin statements to end of ensure statements
            // - If there are no begin statements: span from ensure keyword to end of ensure statements
            const uint8_t *start = prismStatements.empty() ? ensureNode->ensure_keyword_loc.start
                                                           : beginNode->statements->base.location.start;

            const uint8_t *end;
            if (ensureNode->statements) {
                // If there are ensure statements, always extend to include them
                end = ensureNode->statements->base.location.end;
            } else if (!prismStatements.empty()) {
                // No ensure statements but there are begin statements
                end = beginNode->statements->base.location.end;
            } else {
                // No ensure statements and no begin statements
                end = ensureNode->ensure_keyword_loc.end;
            }

            auto loc = translateLoc(start, end);

            if (!directlyDesugar || !hasExpr(bodyNode, ensureBody)) {
                translatedEnsure = make_unique<parser::Ensure>(loc, move(bodyNode), move(ensureBody));
            } else {
                // Build ast::Rescue expression with ensure field set
                // When there's no rescue clause, create a new Rescue with empty rescue cases
                ast::ExpressionPtr bodyExpr;
                bodyExpr = (bodyNode != nullptr) ? bodyNode->takeDesugaredExpr() : ast::MK::EmptyTree();

                ast::ExpressionPtr ensureExpr =
                    (ensureBody != nullptr) ? ensureBody->takeDesugaredExpr() : ast::MK::EmptyTree();

                // Create ast::Rescue with empty rescue cases
                ast::Rescue::RESCUE_CASE_store emptyCases;
                auto emptyElseClause = ast::MK::EmptyTree();
                auto rescueExpr = ast::make_expression<ast::Rescue>(loc, move(bodyExpr), move(emptyCases),
                                                                    move(emptyElseClause), move(ensureExpr));
                translatedEnsure =
                    make_node_with_expr<parser::Ensure>(move(rescueExpr), loc, move(bodyNode), move(ensureBody));
            }
        }

        statements.emplace_back(move(translatedEnsure));
    } else if (translatedRescue != nullptr) {
        // Handle `begin ... rescue ... end` and `begin ... rescue ... else ... end`
        statements.emplace_back(move(translatedRescue));
    } else if (beginNode->statements != nullptr) {
        // Handle just `begin ... end` without ensure or rescue
        statements = translateMulti(beginNode->statements->body);
    }

    return statements;
}

// Translates the given Prism Statements Node into a `parser::Begin` node or an inlined `parser::Node`.
// @param inlineIfSingle If enabled and there's 1 child node, we skip the `Begin` and just return the one `parser::Node`
// @param overrideLocation If provided, use this location for the Begin node instead of the statements node location
unique_ptr<parser::Node> Translator::translateStatements(pm_statements_node *stmtsNode, bool inlineIfSingle,
                                                         core::LocOffsets overrideLocation) {
    if (stmtsNode == nullptr)
        return nullptr;

    // For a single statement, do not create a `Begin` node and just return the statement, if that's enabled.
    if (inlineIfSingle && stmtsNode->body.size == 1) {
        return translate(stmtsNode->body.nodes[0]);
    }

    // For multiple statements, convert each statement and add them to the body of a Begin node
    parser::NodeVec sorbetStmts = translateMulti(stmtsNode->body);

    core::LocOffsets beginNodeLoc;
    if (overrideLocation.exists()) {
        beginNodeLoc = overrideLocation;
    } else if (!sorbetStmts.empty()) {
        auto prismStatements = absl::MakeSpan(stmtsNode->body.nodes, stmtsNode->body.size);
        ENFORCE(!prismStatements.empty());
        ENFORCE(prismStatements.size() == sorbetStmts.size());

        // Cover the locations spanned from the first to the last statements.
        // This can be different from the `stmtsNode->base.location`,
        // because of the special case (handled by `startLoc()` and `endLoc()`).
        beginNodeLoc = translateLoc(startLoc(prismStatements.front()), endLoc(prismStatements.back()));
    } else {
        beginNodeLoc = translateLoc(stmtsNode->base.location);
    }

    if (sorbetStmts.empty()) {
        return make_node_with_expr<parser::Begin>(MK::Nil(beginNodeLoc), beginNodeLoc, NodeVec{});
    }

    if (!directlyDesugar || !hasExpr(sorbetStmts)) {
        return make_unique<parser::Begin>(beginNodeLoc, move(sorbetStmts));
    }

    ast::InsSeq::STATS_store statements;
    statements.reserve(sorbetStmts.size() - 1); // -1 because the `Begin` node stores the last element separately.

    auto end = sorbetStmts.end();
    --end; // Chop one off the end, so we iterate over all but the last element.
    for (auto it = sorbetStmts.begin(); it != end; ++it) {
        auto &statement = *it;
        statements.emplace_back(statement->takeDesugaredExpr());
    };
    auto finalExpr = sorbetStmts.back()->takeDesugaredExpr(); // Process the last element separately.

    auto instructionSequence = MK::InsSeq(beginNodeLoc, move(statements), move(finalExpr));
    return make_node_with_expr<parser::Begin>(move(instructionSequence), beginNodeLoc, move(sorbetStmts));
}

// Helper function for creating if nodes with optional desugaring
unique_ptr<parser::Node> Translator::translateIfNode(core::LocOffsets location, unique_ptr<parser::Node> predicate,
                                                     unique_ptr<parser::Node> ifTrue,
                                                     unique_ptr<parser::Node> ifFalse) {
    if (!directlyDesugar || !hasExpr(predicate, ifTrue, ifFalse)) {
        return make_unique<parser::If>(location, move(predicate), move(ifTrue), move(ifFalse));
    }

    auto condExpr = predicate->takeDesugaredExpr();
    auto thenExpr = ifTrue ? ifTrue->takeDesugaredExpr() : MK::EmptyTree();
    auto elseExpr = ifFalse ? ifFalse->takeDesugaredExpr() : MK::EmptyTree();
    auto ifNode = MK::If(location, move(condExpr), move(thenExpr), move(elseExpr));
    return make_node_with_expr<parser::If>(move(ifNode), location, move(predicate), move(ifTrue), move(ifFalse));
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
// are passed in as `pm_constant_path_node` types, so we need an extra boolean flag to know when to skip the workaround.
//
// Usually returns the `SorbetLHSNode`, but for constant writes and targets,
// it can can return an `LVarLhs` as a workaround in the case of a dynamic constant assignment.
template <typename PrismLhsNode, typename SorbetLHSNode, bool checkForDynamicConstAssign>
unique_ptr<parser::Node> Translator::translateConst(PrismLhsNode *node) {
    static_assert(is_same_v<SorbetLHSNode, parser::Const> || is_same_v<SorbetLHSNode, parser::ConstLhs>,
                  "Invalid LHS type. Must be one of `parser::Const` or `parser::ConstLhs`.");

    // Constant name might be unset, e.g. `::`.
    if (node->name == PM_CONSTANT_ID_UNSET) {
        auto location = translateLoc(node->base.location);
        auto expr = MK::UnresolvedConstant(location, MK::EmptyTree(), core::Names::empty());
        return make_node_with_expr<SorbetLHSNode>(move(expr), location, nullptr, core::Names::empty());
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
            auto expr = MK::Local(location, core::Names::dynamicConstAssign());
            return make_node_with_expr<LVarLhs>(move(expr), location, core::Names::dynamicConstAssign());
        }
    }

    auto location = translateLoc(node->base.location);
    auto constantName = ctx.state.enterNameConstant(name);

    auto constexpr isConstantPath = is_same_v<PrismLhsNode, pm_constant_path_target_node> ||
                                    is_same_v<PrismLhsNode, pm_constant_path_write_node> ||
                                    is_same_v<PrismLhsNode, pm_constant_path_node>;

    unique_ptr<parser::Node> parent;
    ast::ExpressionPtr parentExpr = nullptr;

    if constexpr (isConstantPath) { // Handle constant paths, has a parent node that needs translation.
        if (auto *prismParentNode = node->parent) {
            // This constant reference is chained onto another constant reference.
            // E.g. given `A::B::C`, if `node` is pointing to the root, `A::B` is the `parent`, and `C` is the `name`.
            //   A::B::C
            //    /    \
            //  A::B   ::C
            //  /  \
            // A   ::B
            parent = translate(prismParentNode);
            parentExpr = parent ? parent->takeDesugaredExpr() : nullptr;
        } else { // This is the root of a fully qualified constant reference, like `::A`.
            auto delimiterLoc = translateLoc(node->delimiter_loc); // The location of the `::`
            parent = make_unique<parser::Cbase>(delimiterLoc);
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
        parent = nullptr;
        parentExpr = MK::EmptyTree();
    }

    if (parentExpr == nullptr) {
        parentExpr = MK::EmptyTree();
    }

    ast::ExpressionPtr desugaredExpr = MK::UnresolvedConstant(location, move(parentExpr), constantName);
    return make_node_with_expr<SorbetLHSNode>(move(desugaredExpr), location, move(parent), constantName);
}

core::NameRef Translator::translateConstantName(pm_constant_id_t constant_id) {
    return ctx.state.enterNameUTF8(parser.resolveConstant(constant_id));
}

core::NameRef Translator::nextUniqueParserName(core::NameRef original) {
    return ctx.state.freshNameUnique(core::UniqueNameKind::Parser, original, ++parserUniqueCounter);
}

core::NameRef Translator::nextUniqueDesugarName(core::NameRef original) {
    ENFORCE(directlyDesugar, "This shouldn't be called if we're not directly desugaring.");
    return ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, original, ++desugarUniqueCounter);
}

// Translate the options from a Regexp literal, if any. E.g. the `i` in `/foo/i`
// Had to widen the type from `parser::Assign` to `parser::Node` to handle `make_node_with_expr` correctly.
// TODO: narrow the type back after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
unique_ptr<parser::Node> Translator::translateRegexpOptions(pm_location_t closingLoc) {
    ENFORCE(closingLoc.start && closingLoc.end);

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
    auto prismLoc = pm_location_t{.start = closingLoc.start + 1, .end = closingLoc.end};
    auto location = translateLoc(prismLoc);

    string_view options = sliceLocation(prismLoc);

    if (!directlyDesugar) {
        return make_unique<parser::Regopt>(location, options);
    }

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
    auto flagsExpr = MK::Int(location, flags);
    return make_node_with_expr<parser::Regopt>(move(flagsExpr), location, options);
}

// Translate an unescaped string from a Regexp literal
unique_ptr<parser::Node> Translator::translateRegexp(core::LocOffsets location, core::LocOffsets contentLoc,
                                                     pm_string_t content, pm_location_t closingLoc) {
    // Sorbet's Regexp can have multiple nodes, e.g. for a `PM_INTERPOLATED_REGULAR_EXPRESSION_NODE`,
    // but we'll only have up to one String node here for this non-interpolated Regexp.
    parser::NodeVec parts;
    auto source = parser.extractString(&content);
    if (!source.empty()) {
        if (directlyDesugar) {
            // Create a String node with its desugared expression
            auto name = ctx.state.enterNameUTF8(source);
            auto expr = MK::String(location, name);
            auto sourceStringNode = make_node_with_expr<parser::String>(move(expr), contentLoc, name);
            parts.emplace_back(move(sourceStringNode));
        } else {
            auto sourceStringNode = make_unique<parser::String>(location, ctx.state.enterNameUTF8(source));
            parts.emplace_back(move(sourceStringNode));
        }
    }

    auto options = translateRegexpOptions(closingLoc);

    if (!directlyDesugar) {
        return make_unique<parser::Regexp>(location, move(parts), move(options));
    }

    ast::ExpressionPtr pattern;
    if (parts.empty()) {
        pattern = MK::String(location, core::Names::empty());
    } else {
        pattern = parts[0]->takeDesugaredExpr();
    }
    auto optsExpr = options->takeDesugaredExpr();

    auto cnst = MK::Constant(location, core::Symbols::Regexp());

    // Desugar `/ foo / i` to `::Regexp.new("foo", option_flags_int)`
    auto expr = MK::Send2(location, move(cnst), core::Names::new_(), location.copyWithZeroLength(), move(pattern),
                          move(optsExpr));

    return make_node_with_expr<parser::Regexp>(move(expr), location, move(parts), move(options));
}

string_view Translator::sliceLocation(pm_location_t loc) const {
    return cast_prism_string(loc.start, loc.end - loc.start);
}

// Creates a `parser::Mlhs` for either a `PM_MULTI_WRITE_NODE` or `PM_MULTI_TARGET_NODE`.
template <typename PrismNode>
unique_ptr<parser::Mlhs> Translator::translateMultiTargetLhs(PrismNode *node, core::LocOffsets location) {
    static_assert(
        is_same_v<PrismNode, pm_multi_target_node> || is_same_v<PrismNode, pm_multi_write_node>,
        "Translator::translateMultiTarget can only be used for PM_MULTI_TARGET_NODE and PM_MULTI_WRITE_NODE.");

    // Left-hand side of the assignment
    auto prismLefts = absl::MakeSpan(node->lefts.nodes, node->lefts.size);
    auto prismRights = absl::MakeSpan(node->rights.nodes, node->rights.size);
    auto prismSplat = node->rest;

    NodeVec sorbetLhs{};
    sorbetLhs.reserve(prismLefts.size() + prismRights.size() + (prismSplat != nullptr ? 1 : 0));

    translateMultiInto(sorbetLhs, prismLefts);

    if (prismSplat != nullptr) {
        switch (PM_NODE_TYPE(prismSplat)) {
            case PM_SPLAT_NODE: {
                // This requires separate handling from the `PM_SPLAT_NODE` because it
                // has a different Sorbet node type, `parser::SplatLhs`
                auto splatNode = down_cast<pm_splat_node>(prismSplat);
                auto location = translateLoc(splatNode->base.location);
                auto expression = splatNode->expression;

                if (expression != nullptr && PM_NODE_TYPE_P(expression, PM_REQUIRED_PARAMETER_NODE)) {
                    auto requiredParamNode = down_cast<pm_required_parameter_node>(expression);
                    auto name = translateConstantName(requiredParamNode->name);
                    sorbetLhs.emplace_back(
                        make_unique<parser::RestParam>(location, name, translateLoc(requiredParamNode->base.location)));
                } else {
                    sorbetLhs.emplace_back(make_unique<parser::SplatLhs>(location, move(translate(expression))));
                }

                break;
            }
            case PM_IMPLICIT_REST_NODE:
                // No-op, because Sorbet's parser infers this from just having an `Mlhs`.
                break;
            default:
                unreachable("Unexpected rest node type. Expected only PM_SPLAT_NODE or PM_IMPLICIT_REST_NODE.");
        }
    }

    translateMultiInto(sorbetLhs, prismRights);

    return make_unique<parser::Mlhs>(location, move(sorbetLhs));
}

// Extracts the desugared expressions out of a "scope" (class/sclass/module) body.
// The body can be a Begin node comprising multiple statements, or a single statement.
// Return nullopt if the body does not have all of its expressions desugared.
// TODO: make the return non-optional after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
optional<ast::ClassDef::RHS_store> Translator::desugarScopeBodyToRHSStore(pm_node *prismBodyNode,
                                                                          unique_ptr<parser::Node> &scopeBody) {
    ENFORCE(directlyDesugar, "desugarScopeBodyToRHSStore should only be called when direct desugaring is enabled");

    if (scopeBody == nullptr) { // Empty body
        ast::ClassDef::RHS_store result;
        result.emplace_back(MK::EmptyTree());
        return result;
    }

    ENFORCE(PM_NODE_TYPE_P(prismBodyNode, PM_STATEMENTS_NODE));

    if (1 < down_cast<pm_statements_node>(prismBodyNode)->body.size) { // Handle multi-statement body
        if (!hasExpr(scopeBody)) {
            return nullopt;
        }

        auto beginExpr = scopeBody->takeDesugaredExpr();

        auto insSeqExpr = ast::cast_tree<ast::InsSeq>(beginExpr);
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
        if (!hasExpr(scopeBody)) {
            return nullopt;
        }

        ast::ClassDef::RHS_store result;
        result.emplace_back(scopeBody->takeDesugaredExpr());
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
                                      core::NameRef enclosingBlockParamName) const {
    auto resetDesugarUniqueCounter = true;
    auto isInModule = this->isInModule && !isSingletonMethod;
    return Translator(*this, resetDesugarUniqueCounter, methodLoc, methodName, enclosingBlockParamName, isInModule,
                      this->isInAnyBlock);
}

Translator Translator::enterBlockContext() const {
    auto resetDesugarUniqueCounter = false; // Blocks inherit their parent's numbering
    auto isInAnyBlock = true;
    return Translator(*this, resetDesugarUniqueCounter, this->enclosingMethodLoc, this->enclosingMethodName,
                      this->enclosingBlockParamName, this->isInModule, isInAnyBlock);
}

Translator Translator::enterModuleContext() const {
    auto resetDesugarUniqueCounter = true;
    auto isInModule = true;
    auto isInAnyBlock = false; // Blocks never persist across a class/module boundary
    return Translator(*this, resetDesugarUniqueCounter, this->enclosingMethodLoc, this->enclosingMethodName,
                      this->enclosingBlockParamName, isInModule, isInAnyBlock);
}

Translator Translator::enterClassContext() const {
    auto resetDesugarUniqueCounter = true;
    auto isInModule = false;
    auto isInAnyBlock = false; // Blocks never persist across a class/module boundary
    return Translator(*this, resetDesugarUniqueCounter, this->enclosingMethodLoc, this->enclosingMethodName,
                      this->enclosingBlockParamName, isInModule, isInAnyBlock);
}

void Translator::reportError(core::LocOffsets loc, const string &message) const {
    auto errorLoc = core::Loc(ctx.file, loc);
    if (auto e = ctx.state.beginError(errorLoc, core::errors::Parser::ParserError)) {
        e.setHeader("{}", message);
    }
}
}; // namespace sorbet::parser::Prism
