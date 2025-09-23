#include "Translator.h"
#include "Helpers.h"

#include "ast/Helpers.h"
#include "ast/Trees.h"
#include "ast/desugar/DuplicateHashKeyCheck.h"
#include "core/errors/desugar.h"

#include "absl/strings/str_replace.h"

template class std::unique_ptr<sorbet::parser::Node>;

using namespace std;

namespace sorbet::parser::Prism {

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

// Extract the content and location of a Symbol node.
// This is handy for `desugarSymbolProc`, where it saves us from needing to dig and
// cast to extract this info out of an `ast::Literal`.
pair<core::NameRef, core::LocOffsets> Translator::translateSymbol(pm_symbol_node *symbol) {
    auto location = translateLoc(symbol->base.location);

    auto unescaped = &symbol->unescaped;
    // TODO: can these have different encodings?
    auto content = ctx.state.enterNameUTF8(parser.extractString(unescaped));

    // If the opening location is null, the symbol is used as a key with a colon postfix, like `{a: 1}`
    // In those cases, the location should not include the colon.
    if (symbol->opening_loc.start == nullptr) {
        location = translateLoc(symbol->value_loc);
    }

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
        auto replaceWithDynamicConstAssign = true;
        lhs = translateConst<pm_constant_write_node, parser::ConstLhs>(node, replaceWithDynamicConstAssign);
    } else if constexpr (is_same_v<PrismAssignmentNode, pm_constant_path_write_node>) {
        // Handle regular assignment to a constant path, like `A::B::C = 1` or `::C = 1`
        auto replaceWithDynamicConstAssign = true;
        lhs = translateConst<pm_constant_path_node, parser::ConstLhs>(node->target, replaceWithDynamicConstAssign);
    } else {
        // Handle regular assignment to any other kind of LHS.
        auto name = translateConstantName(node->name);
        auto loc = translateLoc(node->name_loc);
        ast::UnresolvedIdent::Kind kind;

        if constexpr (is_same_v<SorbetLHSNode, parser::IVarLhs>) {
            kind = ast::UnresolvedIdent::Kind::Instance;
        } else if constexpr (is_same_v<SorbetLHSNode, parser::GVarLhs>) {
            kind = ast::UnresolvedIdent::Kind::Global;
        } else if constexpr (is_same_v<SorbetLHSNode, parser::CVarLhs>) {
            kind = ast::UnresolvedIdent::Kind::Class;
        } else if constexpr (is_same_v<SorbetLHSNode, parser::LVarLhs>) {
            kind = ast::UnresolvedIdent::Kind::Local;
        } else {
            static_assert(false && sizeof(SorbetLHSNode),
                          "Invalid LHS type. Must be one of `IVarLhs`, `GVarLhs`, `CVarLhs`, or `LVarLhs`.");
        }

        auto expr = ast::make_expression<ast::UnresolvedIdent>(loc, kind, name);
        lhs = make_node_with_expr<SorbetLHSNode>(move(expr), loc, name);
    }

    if (!hasExpr(lhs, rhs)) {
        return make_unique<parser::Assign>(location, move(lhs), move(rhs));
    }

    auto exp = MK::Assign(location, lhs->takeDesugaredExpr(), rhs->takeDesugaredExpr());
    return make_node_with_expr<parser::Assign>(move(exp), location, move(lhs), move(rhs));
}

template <typename PrismAssignmentNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
unique_ptr<SorbetAssignmentNode> Translator::translateOpAssignment(pm_node_t *untypedNode) {
    static_assert(
        is_same_v<SorbetAssignmentNode, parser::OpAsgn> || is_same_v<SorbetAssignmentNode, parser::AndAsgn> ||
            is_same_v<SorbetAssignmentNode, parser::OrAsgn>,
        "Invalid operator node type. Must be one of `parser::OpAssign`, `parser::AndAsgn` or `parser::OrAsgn`.");

    auto node = down_cast<PrismAssignmentNode>(untypedNode);
    auto location = translateLoc(untypedNode->location);

    unique_ptr<parser::Node> lhs;
    auto rhs = translate(node->value);

    // Various node types need special handling to construct their corresponding Sorbet LHS nodes.
    if constexpr (is_same_v<PrismAssignmentNode, pm_index_operator_write_node> ||
                  is_same_v<PrismAssignmentNode, pm_index_and_write_node> ||
                  is_same_v<PrismAssignmentNode, pm_index_or_write_node>) {
        // Handle operator assignment to an indexed expression, like `a[0] += 1`
        auto openingLoc = translateLoc(node->opening_loc);
        auto lBracketLoc = core::LocOffsets{openingLoc.beginLoc, openingLoc.endLoc - 1};

        auto receiver = translate(node->receiver);
        auto args = translateArguments(node->arguments, up_cast(node->block));
        lhs =
            make_unique<parser::Send>(location, move(receiver), core::Names::squareBrackets(), lBracketLoc, move(args));
    } else if constexpr (is_same_v<PrismAssignmentNode, pm_constant_operator_write_node> ||
                         is_same_v<PrismAssignmentNode, pm_constant_and_write_node> ||
                         is_same_v<PrismAssignmentNode, pm_constant_or_write_node>) {
        // Handle operator assignment to a "plain" constant, like `A += 1`
        auto replaceWithDynamicConstAssign = true;
        lhs = translateConst<PrismAssignmentNode, parser::ConstLhs>(node, replaceWithDynamicConstAssign);
    } else if constexpr (is_same_v<PrismAssignmentNode, pm_constant_path_operator_write_node> ||
                         is_same_v<PrismAssignmentNode, pm_constant_path_and_write_node> ||
                         is_same_v<PrismAssignmentNode, pm_constant_path_or_write_node>) {
        // Handle operator assignment to a constant path, like `A::B::C += 1` or `::C += 1`
        lhs = translateConst<pm_constant_path_node, parser::ConstLhs>(node->target);
    } else if constexpr (is_same_v<SorbetLHSNode, parser::Send> || is_same_v<SorbetLHSNode, parser::CSend>) {
        // Handle operator assignment to the result of a method call, like `a.b += 1`
        auto name = translateConstantName(node->read_name);
        auto receiver = translate(node->receiver);
        auto messageLoc = translateLoc(node->message_loc);
        lhs = make_unique<SorbetLHSNode>(location, move(receiver), name, messageLoc, NodeVec{});
    } else {
        // Handle regular assignment to any other kind of LHS.
        auto nameLoc = translateLoc(node->name_loc);
        auto name = translateConstantName(node->name);
        lhs = make_unique<SorbetLHSNode>(nameLoc, name);
    }

    if constexpr (is_same_v<SorbetAssignmentNode, parser::OpAsgn>) {
        // `OpAsgn` assign needs more information about the specific operator here, so it gets special handling here.
        auto opLoc = translateLoc(node->binary_operator_loc);
        auto op = translateConstantName(node->binary_operator);

        return make_unique<parser::OpAsgn>(location, move(lhs), op, opLoc, move(rhs));
    } else {
        // `AndAsgn` and `OrAsgn` are specific to a single operator, so don't need any extra information like `OpAsgn`.
        static_assert(is_same_v<SorbetAssignmentNode, parser::AndAsgn> ||
                      is_same_v<SorbetAssignmentNode, parser::OrAsgn>);

        return make_unique<SorbetAssignmentNode>(location, move(lhs), move(rhs));
    }
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

            auto locZeroLen = location.copyWithZeroLength();

            ast::Array::ENTRY_store elems;
            elems.reserve(sorbetElements.size());
            ExpressionPtr lastMerge;
            for (auto &stat : sorbetElements) {
                if (parser::NodeWithExpr::isa_node<parser::Splat>(stat.get()) ||
                    parser::NodeWithExpr::isa_node<parser::ForwardedRestArg>(stat.get())) {
                    // Desugar [a, *x, remaining] into a.concat(<splat>(x)).concat(remaining)

                    // The Splat was already desugared to Send{Magic.splat(arg)} with the splat's own location.
                    // But for array literals, we want the splat to have the array's location to match
                    // the legacy parser's behavior (important for error messages and hover).
                    auto splatExpr = stat->takeDesugaredExpr();

                    // The parser::Send case makes a fake parser::Array with locZeroLen to hide callWithSplat
                    // methods from hover. Using the array's loc means that we will get a zero-length loc for
                    // the Splat in that case, and non-zero if there was a real Array literal.
                    if (auto send = ast::cast_tree<ast::Send>(splatExpr)) {
                        ENFORCE(send->numPosArgs() == 1, "Splat Send should have exactly 1 argument");
                        // Extract the argument from the old Send and create a new one with array's location
                        splatExpr = MK::Splat(location, move(send->getPosArg(0)));
                    }
                    auto var = move(splatExpr);
                    if (elems.empty()) {
                        if (lastMerge != nullptr) {
                            lastMerge =
                                MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(var));
                        } else {
                            lastMerge = move(var);
                        }
                    } else {
                        ExpressionPtr current = MK::Array(location, move(elems));
                        /* reassign instead of clear to work around https://bugs.llvm.org/show_bug.cgi?id=37553 */
                        elems = ast::Array::ENTRY_store();
                        if (lastMerge != nullptr) {
                            lastMerge =
                                MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(current));
                        } else {
                            lastMerge = move(current);
                        }
                        lastMerge = MK::Send1(location, move(lastMerge), core::Names::concat(), locZeroLen, move(var));
                    }
                } else {
                    elems.emplace_back(stat->takeDesugaredExpr());
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

            return make_node_with_expr<parser::Array>(move(res), location, move(sorbetElements));
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
            std::tie(params, std::ignore) = translateParametersNode(paramsNode->parameters);

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
            if (PM_NODE_FLAG_P(node, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                return translateOpAssignment<pm_call_and_write_node, parser::AndAsgn, parser::CSend>(node);
            } else {
                return translateOpAssignment<pm_call_and_write_node, parser::AndAsgn, parser::Send>(node);
            }
        }
        case PM_CALL_NODE: { // A method call like `a.b()` or `a&.b()`
            auto callNode = down_cast<pm_call_node>(node);

            auto loc = location;

            auto constantNameString = parser.resolveConstant(callNode->name);
            auto receiver = translate(callNode->receiver);

            core::LocOffsets messageLoc;

            // When the message is empty, like `foo.()`, the message location is the same as the call operator location
            if (callNode->message_loc.start == nullptr && callNode->message_loc.end == nullptr) {
                messageLoc = translateLoc(callNode->call_operator_loc);
            } else {
                messageLoc = translateLoc(callNode->message_loc);
            }

            // Handle `~[Integer]`, like `~42`
            // Unlike `-[Integer]`, Prism treats `~[Integer]` as a method call
            // But Sorbet's legacy parser treats both `~[Integer]` and `-[Integer]` as integer literals
            if (constantNameString == "~" && PM_NODE_TYPE_P(callNode->receiver, PM_INTEGER_NODE)) {
                string valueString(sliceLocation(callNode->base.location));

                if (!directlyDesugar) {
                    return make_unique<parser::Integer>(location, move(valueString));
                }

                // The purely integer part of it, not including the `~`
                auto integerExpr = receiver->takeDesugaredExpr();
                ENFORCE(integerExpr != nullptr, "All Integer nodes should have been desugared already");

                // Model this as an Integer in the parse tree, but desugar to a method call like `42.~()`
                auto sendNode = MK::Send0(loc, move(integerExpr), core::Names::tilde(), loc.copyEndWithZeroLength());
                return make_node_with_expr<parser::Integer>(move(sendNode), location, move(valueString));
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
            auto hasFwdRestArg = false; // true if the call contains an anonymous forwarded rest arg like `foo(*rest)`
            auto hasSplat = false;      // true if the call contains a splatted expression like `foo(*a)`
            unique_ptr<parser::Hash> kwargsHash;
            auto kwargsHashHasExpr = true; // true if we can directly desugar the kwargs Hash, if any.
            if (auto *prismArgsNode = callNode->arguments) {
                auto prismArgs = absl::MakeSpan(prismArgsNode->arguments.nodes, prismArgsNode->arguments.size);

                // Pop the Kwargs Hash off the end of the arguments, if there is one.
                if (!prismArgs.empty() && PM_NODE_TYPE_P(prismArgs.back(), PM_KEYWORD_HASH_NODE)) {
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
            auto methodName = MK::Symbol(location.copyWithZeroLength(), name);

            if (PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                // Handle conditional send, e.g. `a&.b`

                if (prismBlock && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                    // PM_BLOCK_ARGUMENT_NODE models the `&b` in `a.map(&b)`,
                    // but not a literal block with `{ ... }` or `do ... end`

                    auto blockPassNode = translate(prismBlock);
                    args.emplace_back(move(blockPassNode));
                }

                sendNode = make_unique<parser::CSend>(loc, move(receiver), name, messageLoc, move(args));

                // TODO: Direct desugaring support for conditional sends is not implemented yet.

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

            // Regular send, e.g. `a.b`

            // Method defs are really complex, and we're building support for different kinds of arguments bit
            // by bit. This bool is true when this particular method call is supported by our desugar logic.
            auto supportedCallType = constantNameString != "block_given?" && kwargsHashHasExpr && !hasFwdArgs &&
                                     !hasFwdRestArg && !hasSplat && hasExpr(receiver, args);

            unique_ptr<parser::Node> blockBody;       // e.g. `123` in `foo { |x| 123 }`
            unique_ptr<parser::Node> blockParameters; // e.g. `|x|` in `foo { |x| 123 }`
            ast::MethodDef::PARAMS_store blockParamsStore;
            ast::InsSeq::STATS_store blockStatsStore;
            unique_ptr<parser::Node> blockPassNode;
            ast::ExpressionPtr blockPassArg;
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

                                if (paramsNode->parameters == nullptr) {
                                    // This can happen if the block declares block-local variables, but no parameters.
                                    // e.g. `foo { |; block_local_var| ... }`

                                    auto location = translateLoc(paramsNode->base.location);

                                    // TODO: future follow up, ensure we add the block local variables ("shadowargs"),
                                    // if any.
                                    blockParameters = make_unique<parser::Params>(location, NodeVec{});
                                    didDesugarBlockParams = true;
                                } else {
                                    unique_ptr<parser::Params> params;
                                    std::tie(params, std::ignore) = translateParametersNode(paramsNode->parameters);

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
                                auto location = translateLoc(numberedParamsNode->base.location);

                                auto paramCount = numberedParamsNode->maximum;

                                NodeVec params;
                                params.reserve(paramCount);

                                for (auto i = 1; i <= paramCount; i++) {
                                    auto name = ctx.state.enterNameUTF8("_" + to_string(i));

                                    // The location is arbitrary and not really used, since these aren't explicitly
                                    // written in the source.
                                    auto expr = MK::Local(location, name);
                                    auto paramNode = make_node_with_expr<parser::LVar>(move(expr), location, name);
                                    params.emplace_back(move(paramNode));
                                }

                                std::tie(blockParamsStore, blockStatsStore, didDesugarBlockParams) =
                                    desugarParametersNode(params, attemptToDesugarBlockParams);

                                blockParameters = make_unique<parser::NumParams>(location, move(params));

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
                        // Replace an anonymous block pass like `f(&)` with a local variable reference, like
                        // `f(&&)`.
                        blockPassArg = MK::Local(location, core::Names::ampersand());
                        supportedBlock = true;
                    }
                }
            } else {
                // There is no block, so we support direct desugaring of this method call.
                supportedBlock = true;
            }

            supportedCallType &= supportedBlock;

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

                sendNode = make_unique<parser::Send>(loc, move(receiver), name, messageLoc, move(args));

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
                receiverExpr = MK::Self(loc.copyWithZeroLength());
            } else {
                receiverExpr = receiver->takeDesugaredExpr();
            }

            // Unsupported nodes are desugared to an empty tree.
            // Treat them as if they were `self` to match `Desugar.cc`.
            // TODO: Clean up after direct desugaring is complete. https://github.com/Shopify/sorbet/issues/671
            if (ast::isa_tree<ast::EmptyTree>(receiverExpr)) {
                receiverExpr = MK::Self(loc.copyWithZeroLength());
                flags.isPrivateOk = true;
            } else {
                flags.isPrivateOk = PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_IGNORE_VISIBILITY);
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
                }

                if (prismBlock && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                    // Add the parser node back into the wq tree, to pass the parser tests.
                    args.emplace_back(move(blockPassNode));
                }

                auto sendExpr = MK::Send(loc, MK::Magic(loc), core::Names::callWithBlockPass(), messageLoc, numPosArgs,
                                         move(magicSendArgs), flags);

                return make_node_with_expr<parser::Send>(move(sendExpr), loc, move(receiver), name, messageLoc,
                                                         move(args));
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
                        auto blockBodyExpr = blockBody == nullptr ? MK::EmptyTree() : blockBody->takeDesugaredExpr();
                        blockExpr = MK::Block(location, move(blockBodyExpr), move(blockParamsStore));
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

            auto expr = MK::Send(location, move(receiverExpr), name, messageLoc, numPosArgs, move(sendArgs), flags);

            sendNode = make_node_with_expr<parser::Send>(move(expr), loc, move(receiver), name, messageLoc, move(args));

            if (prismBlock != nullptr) {
                if (PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE) || blockPassArgIsSymbol) {
                    // In Prism, this is modeled by a `pm_call_node` with a `pm_block_node` as a child,
                    // but the legacy parser inverts this, with a parent "Block" with a child "Send".
                    //
                    // Note: The legacy parser doesn't treat block pass arguments this way.
                    //       It just puts them at the end of the arguments list,
                    //       which is why we checked for `PM_BLOCK_NODE` specifically here.

                    return make_node_with_expr<parser::Block>(sendNode->takeDesugaredExpr(), sendNode->loc,
                                                              move(sendNode), move(blockParameters), move(blockBody));
                } else {
                    unreachable("Found a {} block type, which is not implemented yet ",
                                pm_node_type_to_str(PM_NODE_TYPE(prismBlock)));
                }
            }

            return sendNode;
        }
        case PM_CALL_OPERATOR_WRITE_NODE: { // Compound assignment to a method call, e.g. `a.b += 1`
            if (PM_NODE_FLAG_P(node, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                return translateOpAssignment<pm_call_operator_write_node, parser::OpAsgn, parser::CSend>(node);
            } else {
                return translateOpAssignment<pm_call_operator_write_node, parser::OpAsgn, parser::Send>(node);
            }
        }
        case PM_CALL_OR_WRITE_NODE: { // Or-assignment to a method call, e.g. `a.b ||= true`
            if (PM_NODE_FLAG_P(node, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) {
                return translateOpAssignment<pm_call_or_write_node, parser::OrAsgn, parser::CSend>(node);
            } else {
                return translateOpAssignment<pm_call_or_write_node, parser::OrAsgn, parser::Send>(node);
            }
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
            auto sorbetConditions = patternTranslateMulti(caseMatchNode->conditions);
            auto elseClause = translate(up_cast(caseMatchNode->else_clause));

            return make_unique<parser::CaseMatch>(location, move(predicate), move(sorbetConditions), move(elseClause));
        }
        case PM_CASE_NODE: { // A classic `case` statement that only uses `when` (and not pattern matching with `in`)
            auto caseNode = down_cast<pm_case_node>(node);

            auto predicate = translate(caseNode->predicate);
            auto sorbetConditions = translateMulti(caseNode->conditions);
            auto elseClause = translate(up_cast(caseNode->else_clause));

            return make_unique<Case>(location, move(predicate), move(sorbetConditions), move(elseClause));
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
            return translateOpAssignment<pm_class_variable_and_write_node, parser::AndAsgn, parser::CVarLhs>(node);
        }
        case PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a class variable, e.g. `@@a += 1`
            return translateOpAssignment<pm_class_variable_operator_write_node, parser::OpAsgn, parser::CVarLhs>(node);
        }
        case PM_CLASS_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a class variable, e.g. `@@a ||= 1`
            return translateOpAssignment<pm_class_variable_or_write_node, parser::OrAsgn, parser::CVarLhs>(node);
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

            return make_unique<parser::CVarLhs>(location, name);
        }
        case PM_CLASS_VARIABLE_WRITE_NODE: { // Regular assignment to a class variable, e.g. `@@a = 1`
            return translateAssignment<pm_class_variable_write_node, parser::CVarLhs>(node);
        }
        case PM_CONSTANT_PATH_AND_WRITE_NODE: { // And-assignment to a constant path, e.g. `A::B &&= false`
            return translateOpAssignment<pm_constant_path_and_write_node, parser::AndAsgn, parser::ConstLhs>(node);
        }
        case PM_CONSTANT_PATH_NODE: { // Part of a constant path, like the `A::B` in `A::B::C`.
            // See`PM_CONSTANT_READ_NODE`, which handles the `::C` part
            auto constantPathNode = down_cast<pm_constant_path_node>(node);

            return translateConst<pm_constant_path_node, parser::Const>(constantPathNode);
        }
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE: { // Compound assignment to a constant path, e.g. `A::B += 1`
            return translateOpAssignment<pm_constant_path_operator_write_node, parser::OpAsgn, parser::ConstLhs>(node);
        }
        case PM_CONSTANT_PATH_OR_WRITE_NODE: { // Or-assignment to a constant path, e.g. `A::B ||= true`
            return translateOpAssignment<pm_constant_path_or_write_node, parser::OrAsgn, parser::ConstLhs>(node);
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
            return translateOpAssignment<pm_constant_and_write_node, parser::AndAsgn, parser::ConstLhs>(node);
        }
        case PM_CONSTANT_OPERATOR_WRITE_NODE: { // Compound assignment to a constant, e.g. `C += 1`
            return translateOpAssignment<pm_constant_operator_write_node, parser::OpAsgn, parser::ConstLhs>(node);
        }
        case PM_CONSTANT_OR_WRITE_NODE: { // Or-assignment to a constant, e.g. `C ||= true`
            return translateOpAssignment<pm_constant_or_write_node, parser::OrAsgn, parser::ConstLhs>(node);
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

            if (rparenLoc.start != nullptr && rparenLoc.end != nullptr) {
                declLoc = declLoc.join(translateLoc(defNode->rparen_loc));
            }

            auto receiver = translate(defNode->receiver); // The singleton receiver, like `self` in `self.foo()`
            auto name = translateConstantName(defNode->name);

            auto isSingletonMethod = receiver != nullptr;

            unique_ptr<parser::Params> params;
            core::NameRef enclosingBlockParamName;
            if (defNode->parameters != nullptr) {
                std::tie(params, enclosingBlockParamName) = translateParametersNode(defNode->parameters);
            } else {
                if (rparenLoc.start != nullptr) {
                    // The definition has no parameters but still has parentheses, e.g. `def foo(); end`
                    // In this case, Sorbet's legacy parser will still hold an empty Args node
                    params = make_unique<parser::Params>(location, NodeVec{});
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

            // Desugar to `::Magic.defined_instance_var(ivar)` or `::Magic.defined_class_var(cvar)`
            if (auto *ivar = parser::NodeWithExpr::cast_node<parser::IVar>(arg.get())) {
                auto sym = MK::Symbol(arg->loc, ivar->name);
                auto expr = MK::Send1(arg->loc, MK::Magic(arg->loc), core::Names::definedInstanceVar(),
                                      location.copyWithZeroLength(), move(sym));
                return make_node_with_expr<parser::Defined>(move(expr), location.join(arg->loc), move(arg));
            }

            if (auto *cvar = parser::NodeWithExpr::cast_node<parser::CVar>(arg.get())) {
                auto sym = MK::Symbol(arg->loc, cvar->name);
                auto expr = MK::Send1(arg->loc, MK::Magic(arg->loc), core::Names::definedClassVar(),
                                      location.copyWithZeroLength(), move(sym));
                return make_node_with_expr<parser::Defined>(move(expr), location.join(arg->loc), move(arg));
            }

            // Desugar to `defined?(A::B::C)` to `::Magic.defined_p("A", "B", "C")`,
            //       or `defined?(::A::B::C)` to `::Magic.defined_p()`.
            ast::Send::ARGS_store args;
            auto constPathNode = arg->takeDesugaredExpr();

            while (!ast::isa_tree<ast::EmptyTree>(constPathNode)) {
                auto lit = ast::cast_tree<ast::UnresolvedConstantLit>(constPathNode);
                if (lit == nullptr) {
                    args.clear();
                    break;
                }
                args.emplace_back(MK::String(lit->loc, lit->cnst));
                constPathNode = move(lit->scope);
            }
            absl::c_reverse(args);

            auto expr = MK::Send(arg->loc, MK::Magic(arg->loc), core::Names::defined_p(), location.copyWithZeroLength(),
                                 args.size(), move(args));
            return make_node_with_expr<parser::Defined>(move(expr), location.join(arg->loc), move(arg));
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
            return translateStatements(stmtsNode, inlineIfSingle);
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

            return make_unique<parser::For>(location, move(variable), move(collection), move(body));
        }
        case PM_FORWARDING_ARGUMENTS_NODE: { // The `...` argument in a method call, like `foo(...)`
            return make_unique<parser::ForwardedArgs>(location);
        }
        case PM_FORWARDING_PARAMETER_NODE: { // The `...` parameter in a method definition, like `def foo(...)`
            // Desugared in desugarParametersNode().
            return make_unique<parser::ForwardArg>(location);
        }
        case PM_FORWARDING_SUPER_NODE: { // `super` with no `(...)`
            auto forwardingSuperNode = down_cast<pm_forwarding_super_node>(node);

            auto expr = MK::ZSuper(location, maybeTypedSuper());
            auto translatedNode = make_node_with_expr<parser::ZSuper>(move(expr), location);

            auto blockArgumentNode = forwardingSuperNode->block;

            if (blockArgumentNode != nullptr) { // always a PM_BLOCK_NODE
                return translateCallWithBlock(up_cast(blockArgumentNode), move(translatedNode));
            }

            return translatedNode;
        }
        case PM_GLOBAL_VARIABLE_AND_WRITE_NODE: { // And-assignment to a global variable, e.g. `$g &&= false`
            return translateOpAssignment<pm_global_variable_and_write_node, parser::AndAsgn, parser::GVarLhs>(node);
        }
        case PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a global variable, e.g. `$g += 1`
            return translateOpAssignment<pm_global_variable_operator_write_node, parser::OpAsgn, parser::GVarLhs>(node);
        }
        case PM_GLOBAL_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a global variable, e.g. `$g ||= true`
            return translateOpAssignment<pm_global_variable_or_write_node, parser::OrAsgn, parser::GVarLhs>(node);
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

            return make_unique<parser::GVarLhs>(location, name);
        }
        case PM_GLOBAL_VARIABLE_WRITE_NODE: { // Regular assignment to a global variable, e.g. `$g = 1`
            return translateAssignment<pm_global_variable_write_node, parser::GVarLhs>(node);
        }
        case PM_HASH_NODE: { // A hash literal, like `{ a: 1, b: 2 }`
            auto kvPairs = translateKeyValuePairs(down_cast<pm_hash_node>(node)->elements);

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

            if (hasSign) {
                value.remove_prefix(1); // Remove the sign
            }

            // Create the desugared Complex call: `Kernel.Complex(0, unsigned_value)`
            auto kernel = MK::Constant(location, core::Symbols::Kernel());
            core::NameRef complexName = core::Names::Constants::Complex().dataCnst(ctx)->original;
            core::NameRef valueName = ctx.state.enterNameUTF8(value);
            auto complexCall = MK::Send2(location, move(kernel), complexName, location.copyWithZeroLength(),
                                         MK::Int(location, 0), MK::String(location, valueName));

            // If there was a sign, wrap in unary operation
            // E.g. desugar `+42` to `42.+()`
            if (hasSign) {
                auto complexNode = make_unique<parser::Complex>(location, string(value));
                core::NameRef unaryOp = (sign == '-') ? core::Names::unaryMinus() : core::Names::unaryPlus();

                auto unarySend = MK::Send0(location, move(complexCall), unaryOp,
                                           core::LocOffsets{location.beginLoc, location.beginLoc + 1});

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
            return translateOpAssignment<pm_index_and_write_node, parser::AndAsgn, void>(node);
        }
        case PM_INDEX_OPERATOR_WRITE_NODE: { // Compound assignment to an index, e.g. `a[i] += 1`
            return translateOpAssignment<pm_index_operator_write_node, parser::OpAsgn, void>(node);
        }
        case PM_INDEX_OR_WRITE_NODE: { // Or-assignment to an index, e.g. `a[i] ||= true`
            return translateOpAssignment<pm_index_or_write_node, parser::OrAsgn, void>(node);
        }
        case PM_INDEX_TARGET_NODE: { // Target of an indirect write to an indexed expression
            // ... like `target[0], target[1] = 1, 2`, `rescue => target[0]`, etc.
            auto indexedTargetNode = down_cast<pm_index_target_node>(node);

            auto openingLoc = translateLoc(indexedTargetNode->opening_loc);                  // The location of `[]=`
            auto lBracketLoc = core::LocOffsets{openingLoc.beginLoc, openingLoc.endLoc - 1}; // Drop the `=`
            auto receiver = translate(indexedTargetNode->receiver);
            auto arguments = translateArguments(indexedTargetNode->arguments, up_cast(indexedTargetNode->block));

            return make_unique<parser::Send>(location, move(receiver), core::Names::squareBracketsEq(), lBracketLoc,
                                             move(arguments));
        }
        case PM_INSTANCE_VARIABLE_AND_WRITE_NODE: { // And-assignment to an instance variable, e.g. `@iv &&= false`
            return translateOpAssignment<pm_instance_variable_and_write_node, parser::AndAsgn, parser::IVarLhs>(node);
        }
        case PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to an instance variable, e.g. `@iv += 1`
            return translateOpAssignment<pm_instance_variable_operator_write_node, parser::OpAsgn, parser::IVarLhs>(
                node);
        }
        case PM_INSTANCE_VARIABLE_OR_WRITE_NODE: { // Or-assignment to an instance variable, e.g. `@iv ||= true`
            return translateOpAssignment<pm_instance_variable_or_write_node, parser::OrAsgn, parser::IVarLhs>(node);
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

            return make_unique<parser::IVarLhs>(location, name);
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

            return make_unique<parser::Regexp>(location, move(parts), move(options));
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

            auto isKwargs =
                PM_NODE_FLAG_P(keywordHashNode, PM_KEYWORD_HASH_NODE_FLAGS_SYMBOL_KEYS) ||
                absl::c_all_of(absl::MakeSpan(keywordHashNode->elements.nodes, keywordHashNode->elements.size),
                               [](const auto *node) {
                                   // Checks if the given node is a keyword hash element based on the standards of
                                   // Sorbet's legacy parser. Based on `Builder::isKeywordHashElement()`

                                   if (PM_NODE_TYPE_P(node, PM_ASSOC_SPLAT_NODE)) {
                                       return true;
                                   }

                                   return false;
                               });

            return make_unique<parser::Hash>(location, isKwargs, move(kvPairs));
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: { // A keyword rest parameter, like `def foo(**kwargs)`
            // This doesn't include `**nil`, which is a `PM_NO_KEYWORDS_PARAMETER_NODE`.
            auto keywordRestParamNode = down_cast<pm_keyword_rest_parameter_node>(node);

            core::NameRef sorbetName;
            if (auto prismName = keywordRestParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named keyword rest parameter, like `def foo(**kwargs)`
                sorbetName = translateConstantName(prismName);
            } else { // An anonymous keyword rest parameter, like `def foo(**)`
                sorbetName = nextUniqueParserName(core::Names::starStar());
            }

            auto kwrestLoc = core::LocOffsets{location.beginPos() + 2, location.endPos()};
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
            return translateOpAssignment<pm_local_variable_and_write_node, parser::AndAsgn, parser::LVarLhs>(node);
        }
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE: { // Compound assignment to a local variable, e.g. `local += 1`
            return translateOpAssignment<pm_local_variable_operator_write_node, parser::OpAsgn, parser::LVarLhs>(node);
        }
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE: { // Or-assignment to a local variable, e.g. `local ||= true`
            return translateOpAssignment<pm_local_variable_or_write_node, parser::OrAsgn, parser::LVarLhs>(node);
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

            auto regex = translateRegexp(matchLastLineNode->unescaped, location, matchLastLineNode->closing_loc);

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

            return translateMultiTargetLhs(multiTargetNode);
        }
        case PM_MULTI_WRITE_NODE: { // Multi-assignment, like `a, b = 1, 2`
            auto multiWriteNode = down_cast<pm_multi_write_node>(node);

            auto multiLhsNode = translateMultiTargetLhs(multiWriteNode);
            auto rhsValue = translate(multiWriteNode->value);

            return make_unique<parser::Masgn>(location, move(multiLhsNode), move(rhsValue));
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
            auto numberedParamsNode = down_cast<pm_numbered_parameters_node>(node);

            auto paramCount = numberedParamsNode->maximum;

            NodeVec params;
            params.reserve(paramCount);

            for (auto i = 1; i <= paramCount; i++) {
                auto name = ctx.state.enterNameUTF8("_" + to_string(i));

                // The location is arbitrary and not really used, since these aren't explicitly written in the source.
                auto expr = MK::Local(location, name);
                auto paramNode = make_node_with_expr<parser::LVar>(move(expr), location, name);
                params.emplace_back(move(paramNode));
            }

            return make_unique<parser::NumParams>(location, move(params));
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

            auto inlineIfSingle = false;
            // Override the begin node location to be the parentheses location instead of the statements location
            return translateStatements(down_cast<pm_statements_node>(stmtsNode), inlineIfSingle,
                                       parensNode->base.location);
        }
        case PM_PRE_EXECUTION_NODE: {
            auto preExecutionNode = down_cast<pm_pre_execution_node>(node);
            auto body = translateStatements(preExecutionNode->statements);
            return make_unsupported_node<parser::Preexe>(location, move(body));
        }
        case PM_PROGRAM_NODE: { // The root node of the parse tree, representing the entire program
            pm_program_node *programNode = down_cast<pm_program_node>(node);

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

            return translateRegexp(regexNode->unescaped, location, regexNode->closing_loc);
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
            NodeVec cases;
            // In rescue modifiers, users can't specify exceptions and the variable name so they're null
            cases.emplace_back(make_unique<parser::Resbody>(location, nullptr, nullptr, move(rescue)));

            return make_unique<parser::Rescue>(location, move(body), move(cases), nullptr);
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
            if (expr == nullptr) { // An anonymous splat like `f(*)`
                return make_unique<parser::ForwardedRestArg>(location);
            } else { // Splatting an expression like `f(*a)`
                return make_unique<parser::Splat>(location, move(expr));
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
        case PM_SUPER_NODE: { // The `super` keyword, like `super`, `super(a, b)`
            auto superNode = down_cast<pm_super_node>(node);

            auto blockArgumentNode = superNode->block;
            NodeVec returnValues;

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

            // Create the backtick send call for the desugared expression
            auto sendBacktick = MK::Send1(location, MK::Self(location), core::Names::backtick(),
                                          location.copyWithZeroLength(), MK::String(location, content));

            // Create the XString NodeVec with a single string node
            NodeVec nodes{};
            nodes.emplace_back(make_node_with_expr<parser::String>(MK::String(location, content), location, content));

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

        case PM_MISSING_NODE:
            // For now, we only report errors when we hit a missing node because we don't want to always report dynamic
            // constant assignment errors
            // TODO: We will improve this in the future when we handle more errored cases
            for (auto &error : parseErrors) {
                // EOF error is always pointed to the very last line of the file, which can't be expressed in Sorbet's
                // error comments
                if (error.id != PM_ERR_UNEXPECTED_TOKEN_CLOSE_CONTEXT) {
                    reportError(translateLoc(error.location), error.message);
                }
            }
            return make_unique<parser::Const>(location, nullptr, core::Names::Constants::ErrorNode());
    }
}

core::LocOffsets Translator::translateLoc(pm_location_t loc) const {
    return parser.translateLocation(loc);
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

            return make_unique<parser::MatchAlt>(location, move(left), move(right));
        }
        case PM_ASSOC_NODE: { // A key-value pair in a Hash pattern, e.g. the `k: v` in `h in { k: v }
            auto assocNode = down_cast<pm_assoc_node>(node);

            auto key = patternTranslate(assocNode->key);
            auto value = patternTranslate(assocNode->value);

            if (PM_NODE_TYPE_P(assocNode->value, PM_IMPLICIT_NODE)) {
                return value;
            }

            return make_unique<parser::Pair>(location, move(key), move(value));
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

            unique_ptr<parser::Node> arrayPattern = nullptr;

            // When the pattern ends with an implicit rest node, we need to return an `ArrayPatternWithTail` instead
            if (prismRestNode != nullptr && PM_NODE_TYPE_P(prismRestNode, PM_IMPLICIT_REST_NODE)) {
                arrayPattern = make_unique<parser::ArrayPatternWithTail>(location, move(sorbetElements));
            } else {
                arrayPattern = make_unique<parser::ArrayPattern>(location, move(sorbetElements));
            }

            if (auto *prismConstant = arrayPatternNode->constant) {
                // An array pattern can start with a constant that matches against a specific type,
                // rather than any value whose `#deconstruct` results are matched by the pattern
                // E.g. the `Point` in `in Point[1, 2]`
                auto sorbetConstant = translate(prismConstant);

                return make_unique<parser::ConstPattern>(location, move(sorbetConstant), move(arrayPattern));
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
                sorbetElements.emplace_back(make_unique<MatchRest>(splatLoc, move(expr)));
            }

            patternTranslateMultiInto(sorbetElements, prismMiddleNodes);

            if (prismTrailingSplat != nullptr && PM_NODE_TYPE_P(prismTrailingSplat, PM_SPLAT_NODE)) {
                // TODO: handle PM_NODE_TYPE_P(prismTrailingSplat, PM_MISSING_NODE)
                auto prismSplatNode = down_cast<pm_splat_node>(prismTrailingSplat);
                auto expr = patternTranslate(prismSplatNode->expression);
                auto splatLoc = translateLoc(prismSplatNode->base.location);
                sorbetElements.emplace_back(make_unique<MatchRest>(splatLoc, move(expr)));
            }

            return make_unique<parser::FindPattern>(location, move(sorbetElements));
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
                        sorbetElements.emplace_back(
                            make_unique<parser::MatchRest>(loc, patternTranslate(assocSplatNode->value)));
                        break;
                    }
                    case PM_NO_KEYWORDS_PARAMETER_NODE: {
                        sorbetElements.emplace_back(make_unique<parser::MatchNilPattern>(loc));
                        break;
                    }
                    default:
                        sorbetElements.emplace_back(patternTranslate(prismRestNode));
                }
            }

            auto hashPattern = make_unique<parser::HashPattern>(location, move(sorbetElements));

            if (auto *prismConstant = hashPatternNode->constant) {
                // A hash pattern can start with a constant that matches against a specific type,
                // rather than any value whose `#deconstruct_keys` results are matched by the pattern
                // E.g. the `Point` in `in Point[x: Integer => 1, y: Integer => 2]`
                auto sorbetConstant = translate(prismConstant);

                return make_unique<parser::ConstPattern>(location, move(sorbetConstant), move(hashPattern));
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
                    sorbetGuard = make_unique<parser::IfGuard>(location, translate(ifNode->predicate));
                } else { // PM_UNLESS_NODE
                    auto unlessNode = down_cast<pm_unless_node>(prismPattern);
                    conditionalStatements = unlessNode->statements;
                    sorbetGuard = make_unique<parser::UnlessGuard>(location, translate(unlessNode->predicate));
                }

                ENFORCE(
                    conditionalStatements->body.size == 1,
                    "In pattern-matching's `in` clause, a conditional (if/unless) guard must have a single statement.");

                sorbetPattern = patternTranslate(conditionalStatements->body.nodes[0]);
            } else {
                sorbetPattern = patternTranslate(prismPattern);
            }

            return make_unique<parser::InPattern>(location, move(sorbetPattern), move(sorbetGuard), move(statements));
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // A variable binding in a pattern, like the `head` in `[head, *tail]`
            auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
            auto name = translateConstantName(localVarTargetNode->name);

            return make_unique<MatchVar>(location, name);
        }
        case PM_PINNED_EXPRESSION_NODE: { // A "pinned" expression, like `^(1 + 2)` in `in ^(1 + 2)`
            auto pinnedExprNode = down_cast<pm_pinned_expression_node>(node);

            auto expr = translate(pinnedExprNode->expression);

            // Sorbet's parser always wraps the pinned expression in a `Begin` node.
            NodeVec statements;
            statements.emplace_back(move(expr));
            auto beginNode = make_node_with_expr<parser::Begin>(MK::Nil(location), location, move(statements));

            return make_unique<Pin>(location, move(beginNode));
        }
        case PM_PINNED_VARIABLE_NODE: { // A "pinned" variable, like `^x` in `in ^x`
            auto pinnedVarNode = down_cast<pm_pinned_variable_node>(node);

            auto variable = translate(pinnedVarNode->variable);

            return make_unique<Pin>(location, move(variable));
        }
        case PM_SPLAT_NODE: { // A splat, like `*a` in an array pattern
            auto prismSplatNode = down_cast<pm_splat_node>(node);
            auto expr = patternTranslate(prismSplatNode->expression);
            return make_unique<MatchRest>(location, move(expr));
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
Translator::translateParametersNode(pm_parameters_node *paramsNode) {
    auto location = translateLoc(paramsNode->base.location);

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

    translateMultiInto(params, requireds);
    translateMultiInto(params, optionals);

    if (paramsNode->rest != nullptr) {
        params.emplace_back(translate(paramsNode->rest));
    }

    translateMultiInto(params, posts);
    translateMultiInto(params, keywords);

    if (auto *prismKwRestNode = paramsNode->keyword_rest) {
        switch (PM_NODE_TYPE(prismKwRestNode)) {
            case PM_KEYWORD_REST_PARAMETER_NODE: // `def foo(**kwargs)`
            case PM_FORWARDING_PARAMETER_NODE: { // `def foo(...)`
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
    if (auto *prismBlockParam = paramsNode->block) {
        if (auto prismName = prismBlockParam->name; prismName != PM_CONSTANT_ID_UNSET) {
            // A named block parameter, like `def foo(&block)`
            enclosingBlockParamName = translateConstantName(prismName);
        } else { // An anonymous block parameter, like `def foo(&)`
            enclosingBlockParamName = nextUniqueParserName(core::Names::ampersand());
        }

        auto blockParamLoc = translateLoc(prismBlockParam->base.location);
        // Drop the `&` before the name of the block parameter.
        blockParamLoc = core::LocOffsets{blockParamLoc.beginPos() + 1, blockParamLoc.endPos()};

        auto blockParamExpr = MK::BlockArg(blockParamLoc, MK::Local(blockParamLoc, enclosingBlockParamName));
        auto blockParamNode =
            make_node_with_expr<parser::Blockarg>(move(blockParamExpr), blockParamLoc, enclosingBlockParamName);

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
            paramsStore.emplace_back(MK::BlockArg(loc, MK::Local(loc, core::Names::fwdBlock())));
        } else {
            paramsStore.emplace_back(param->takeDesugaredExpr());
        }
    }

    return make_tuple(move(paramsStore), move(statsStore), true);
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

    for (auto &pair : prismElements) {
        if (PM_NODE_TYPE_P(pair, PM_ASSOC_SPLAT_NODE)) {
            auto prismSplatNode = down_cast<pm_assoc_splat_node>(pair);
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
            ENFORCE(PM_NODE_TYPE_P(pair, PM_ASSOC_NODE))
            unique_ptr<parser::Node> sorbetKVPair = translate(pair);
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
    unique_ptr<parser::Node> parametersNode;
    unique_ptr<parser::Node> body;
    if (PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_BLOCK_NODE)) {
        auto prismBlockNode = down_cast<pm_block_node>(prismBlockOrLambdaNode);
        parametersNode = translate(prismBlockNode->parameters);
        body = this->enterBlockContext().translate(prismBlockNode->body);
    } else {
        ENFORCE(PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_LAMBDA_NODE))
        auto prismLambdaNode = down_cast<pm_lambda_node>(prismBlockOrLambdaNode);
        parametersNode = translate(prismLambdaNode->parameters);
        body = this->enterBlockContext().translate(prismLambdaNode->body);
    }

    // There was a TODO in the original Desugarer: "the send->block's loc is too big and includes the whole send."
    // We'll keep this behaviour for parity for now.
    // TODO: Switch to using the fixed sendNode loc below after direct desugaring is complete
    // https://github.com/Shopify/sorbet/issues/671
    auto blockLoc = sendNode->loc;

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
unique_ptr<parser::Node> Translator::translateRescue(pm_rescue_node *prismRescueNode, unique_ptr<parser::Node> bodyNode,
                                                     unique_ptr<parser::Node> elseNode) {
    auto rescueLoc = translateLoc(prismRescueNode->base.location);
    NodeVec rescueBodies;

    // Each `rescue` clause generates a `Resbody` node, which is a child of the `Rescue` node.
    for (pm_rescue_node *currentRescueNode = prismRescueNode; currentRescueNode != nullptr;
         currentRescueNode = currentRescueNode->subsequent) {
        // Translate the exception variable (e.g. the `=> e` in `rescue => e`)
        auto var = translate(currentRescueNode->reference);

        // Translate the body of the rescue clause
        auto rescueBody = translateStatements(currentRescueNode->statements);

        // Translate the exceptions being rescued (e.g., `RuntimeError` in `rescue RuntimeError`)
        auto exceptions = translateMulti(currentRescueNode->exceptions);
        auto exceptionsArray =
            exceptions.empty()
                ? nullptr
                : make_unique<parser::Array>(translateLoc(currentRescueNode->base.location), move(exceptions));

        auto resbodyLoc = translateLoc(currentRescueNode->base.location);

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

        rescueBodies.emplace_back(
            make_unique<parser::Resbody>(resbodyLoc, move(exceptionsArray), move(var), move(rescueBody)));
    }

    // The `Rescue` node combines the main body, the rescue clauses, and the else clause.
    return make_unique<parser::Rescue>(rescueLoc, move(bodyNode), move(rescueBodies), move(elseNode));
}

NodeVec Translator::translateEnsure(pm_begin_node *beginNode) {
    auto location = translateLoc(beginNode->base.location);

    NodeVec statements;

    unique_ptr<parser::Node> translatedRescue;
    if (beginNode->rescue_clause != nullptr) {
        // Extract rescue and else nodes from the begin node
        auto bodyNode = translateStatements(beginNode->statements);
        auto elseNode = translate(up_cast(beginNode->else_clause));
        // We need to pass the rescue node to the Ensure node if it exists instead of adding it to the
        // statements
        translatedRescue = translateRescue(beginNode->rescue_clause, move(bodyNode), move(elseNode));
    }

    if (auto *ensureNode = beginNode->ensure_clause) {
        // Handle `begin ... ensure ... end`
        // When both ensure and rescue are present, Sorbet's legacy parser puts the Rescue node inside the
        // Ensure node.
        auto bodyNode = translateStatements(beginNode->statements);
        auto ensureBody = translateStatements(ensureNode->statements);

        unique_ptr<parser::Ensure> translatedEnsure;
        if (translatedRescue != nullptr) {
            translatedEnsure = make_unique<parser::Ensure>(location, move(translatedRescue), move(ensureBody));
        } else {
            translatedEnsure = make_unique<parser::Ensure>(location, move(bodyNode), move(ensureBody));
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
                                                         std::optional<pm_location_t> overrideLocation) {
    if (stmtsNode == nullptr)
        return nullptr;

    // For a single statement, do not create a `Begin` node and just return the statement, if that's enabled.
    if (inlineIfSingle && stmtsNode->body.size == 1) {
        return translate(stmtsNode->body.nodes[0]);
    }

    // For multiple statements, convert each statement and add them to the body of a Begin node
    parser::NodeVec sorbetStmts = translateMulti(stmtsNode->body);

    auto beginLoc = translateLoc(overrideLocation.value_or(stmtsNode->base.location));

    if (sorbetStmts.empty()) {
        return make_node_with_expr<parser::Begin>(MK::Nil(beginLoc), beginLoc, NodeVec{});
    }

    if (!directlyDesugar || !hasExpr(sorbetStmts)) {
        return make_unique<parser::Begin>(beginLoc, move(sorbetStmts));
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

    auto instructionSequence = MK::InsSeq(beginLoc, move(statements), move(finalExpr));
    return make_node_with_expr<parser::Begin>(move(instructionSequence), beginLoc, move(sorbetStmts));
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
template <typename PrismLhsNode, typename SorbetLHSNode>
unique_ptr<parser::Node> Translator::translateConst(PrismLhsNode *node, bool replaceWithDynamicConstAssign) {
    static_assert(is_same_v<SorbetLHSNode, parser::Const> || is_same_v<SorbetLHSNode, parser::ConstLhs>,
                  "Invalid LHS type. Must be one of `parser::Const` or `parser::ConstLhs`.");

    auto location = translateLoc(node->base.location);
    // It's important that in all branches `enterNameUTF8` is called, which `translateConstantName` does,
    // so that the name is available for the rest of the pipeline.
    auto name = translateConstantName(node->name);

    if (this->isInMethodDef() && replaceWithDynamicConstAssign) {
        // Check if this is a dynamic constant assignment (SyntaxError at runtime)
        // This is a copy of a workaround from `Desugar.cc`, which substitues in a fake assignment,
        // so the parsing can continue. See other usages of `dynamicConstAssign` for more details.
        return make_unique<LVarLhs>(location, core::Names::dynamicConstAssign());
    }

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

    if (parentExpr != nullptr) {
        ast::ExpressionPtr desugaredExpr = MK::UnresolvedConstant(location, move(parentExpr), constantName);
        return make_node_with_expr<SorbetLHSNode>(move(desugaredExpr), location, move(parent), constantName);
    } else {
        return make_unique<SorbetLHSNode>(location, move(parent), constantName);
    }
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
unique_ptr<parser::Regopt> Translator::translateRegexpOptions(pm_location_t closingLoc) {
    auto length = closingLoc.end - closingLoc.start;

    string_view options;

    if (length > 0) {
        options = sliceLocation(closingLoc).substr(1); // one character after the closing `/`
    } else {
        options = string_view();
    }

    return make_unique<parser::Regopt>(translateLoc(closingLoc), options);
}

// Translate an unescaped string from a Regexp literal
unique_ptr<parser::Regexp> Translator::translateRegexp(pm_string_t unescaped, core::LocOffsets location,
                                                       pm_location_t closingLoc) {
    // Sorbet's Regexp can have multiple nodes, e.g. for a `PM_INTERPOLATED_REGULAR_EXPRESSION_NODE`,
    // but we'll only have up to one String node here for this non-interpolated Regexp.
    parser::NodeVec parts;
    auto source = parser.extractString(&unescaped);
    if (!source.empty()) {
        auto sourceStringNode = make_unique<parser::String>(location, ctx.state.enterNameUTF8(source));
        parts.emplace_back(move(sourceStringNode));
    }

    auto options = translateRegexpOptions(closingLoc);

    return make_unique<parser::Regexp>(location, move(parts), move(options));
}

string_view Translator::sliceLocation(pm_location_t loc) const {
    return cast_prism_string(loc.start, loc.end - loc.start);
}

// Creates a `parser::Mlhs` for either a `PM_MULTI_WRITE_NODE` or `PM_MULTI_TARGET_NODE`.
template <typename PrismNode> unique_ptr<parser::Mlhs> Translator::translateMultiTargetLhs(PrismNode *node) {
    static_assert(
        is_same_v<PrismNode, pm_multi_target_node> || is_same_v<PrismNode, pm_multi_write_node>,
        "Translator::translateMultiTarget can only be used for PM_MULTI_TARGET_NODE and PM_MULTI_WRITE_NODE.");

    auto location = translateLoc(node->base.location);

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
