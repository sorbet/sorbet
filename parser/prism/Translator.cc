#include "Translator.h"
#include "Helpers.h"

#include "ast/Helpers.h"
#include "ast/Trees.h"

template class std::unique_ptr<sorbet::parser::Node>;

using namespace std;

namespace sorbet::parser::Prism {

using sorbet::ast::MK;

// Returns true if all nodes have a desugared expr or are null.
// Call this with all of a node's children, to check if that node can be desugared.
template <typename... Rest> bool hasExpr(const std::unique_ptr<parser::Node> &first, const Rest &...rest) {
    bool firstHasExpr = first == nullptr || first->hasDesugaredExpr();
    if constexpr (sizeof...(rest) == 0) {
        return firstHasExpr;
    } else {
        return firstHasExpr && hasExpr(rest...);
    }
}

// Check if all nodes in NodeVec are null or have a desugared expr.
bool hasExpr(const parser::NodeVec &nodes) {
    return absl::c_find_if(nodes, [](const auto &node) { return node != nullptr && !node->hasDesugaredExpr(); }) ==
           nodes.end();
}

// Allocates a new `NodeWithExpr` with a pre-computed `ExpressionPtr` AST.
template <typename SorbetNode, typename... TArgs>
unique_ptr<NodeWithExpr> make_node_with_expr(ast::ExpressionPtr desugaredExpr, TArgs &&...args) {
    auto whiteQuarkNode = make_unique<SorbetNode>(std::forward<TArgs>(args)...);
    return make_unique<NodeWithExpr>(move(whiteQuarkNode), move(desugaredExpr));
}

// Indicates that a particular code path should never be reached, with an explanation of why.
// Throws a `sorbet::SorbetException` when triggered to help with debugging.
template <typename... TArgs>
[[noreturn]] void unreachable(fmt::format_string<TArgs...> reasonFormatStr, TArgs &&...args) {
    Exception::raise(reasonFormatStr, forward<TArgs>(args)...);
}

template <typename PrismAssignmentNode, typename SorbetLHSNode>
unique_ptr<parser::Assign> Translator::translateAssignment(pm_node_t *untypedNode) {
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
        lhs = make_unique<SorbetLHSNode>(translateLoc(node->name_loc), name);
    }

    return make_unique<parser::Assign>(location, move(lhs), move(rhs));
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

unique_ptr<parser::Node> Translator::translate(pm_node_t *node) {
    if (node == nullptr)
        return nullptr;

    auto location = translateLoc(node->location);

    switch (PM_NODE_TYPE(node)) {
        case PM_ALIAS_GLOBAL_VARIABLE_NODE: { // // The `alias` keyword used for global vars, like `alias $new $old`
            auto aliasGlobalVariableNode = down_cast<pm_alias_global_variable_node>(node);

            auto newName = translate(aliasGlobalVariableNode->new_name);
            auto oldName = translate(aliasGlobalVariableNode->old_name);

            return make_unique<parser::Alias>(location, move(newName), move(oldName));
        }
        case PM_ALIAS_METHOD_NODE: { // The `alias` keyword, like `alias new_method old_method`
            auto aliasMethodNode = down_cast<pm_alias_method_node>(node);

            auto newName = translate(aliasMethodNode->new_name);
            auto oldName = translate(aliasMethodNode->old_name);

            return make_unique<parser::Alias>(location, move(newName), move(oldName));
        }
        case PM_AND_NODE: { // operator `&&` and `and`
            auto andNode = down_cast<pm_and_node>(node);

            auto left = translate(andNode->left);
            auto right = translate(andNode->right);

            return make_unique<parser::And>(location, move(left), move(right));
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

            return make_unique<parser::Array>(location, move(sorbetElements));
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
        case PM_BACK_REFERENCE_READ_NODE: {
            auto backReferenceReadNode = down_cast<pm_back_reference_read_node>(node);
            auto name = translateConstantName(backReferenceReadNode->name);

            return make_unique<parser::Backref>(location, name);
        }
        case PM_BEGIN_NODE: { // A `begin ... end` block
            auto beginNode = down_cast<pm_begin_node>(node);

            unique_ptr<parser::Node> translatedRescue;
            if (beginNode->rescue_clause != nullptr) {
                // Extract rescue and else nodes from the begin node
                auto bodyNode = translateStatements(beginNode->statements);
                auto elseNode = translate(up_cast(beginNode->else_clause));
                // We need to pass the rescue node to the Ensure node if it exists instead of adding it to the
                // statements
                translatedRescue = translateRescue(beginNode->rescue_clause, move(bodyNode), move(elseNode));
            }

            NodeVec statements;
            if (auto ensureNode = beginNode->ensure_clause; ensureNode != nullptr) {
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

            return make_unique<parser::Kwbegin>(location, move(statements));
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
            return make_unique<parser::Shadowarg>(location, sorbetName);
        }
        case PM_BLOCK_PARAMETER_NODE: { // A block parameter declared at the top of a method, e.g. `def m(&block)`
            auto blockParamNode = down_cast<pm_block_parameter_node>(node);

            core::NameRef sorbetName;
            if (auto prismName = blockParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named block parameter, like `def foo(&block)`
                sorbetName = translateConstantName(prismName);
            } else { // An anonymous block parameter, like `def foo(&)`
                sorbetName = gs.freshNameUnique(core::UniqueNameKind::Parser, core::Names::ampersand(), nextUniqueID());
            }

            auto blockLoc = core::LocOffsets{location.beginPos() + 1, location.endPos()};

            return make_unique<parser::Blockarg>(blockLoc, sorbetName);
        }
        case PM_BLOCK_PARAMETERS_NODE: { // The parameters declared at the top of a PM_BLOCK_NODE
            auto paramsNode = down_cast<pm_block_parameters_node>(node);

            if (paramsNode->parameters == nullptr) {
                return make_unique<parser::Args>(location, NodeVec{});
            }

            // Sorbet's legacy parser inserts locals (Shadowargs) into the block's Args node, along with its other
            // parameters. So we need to extract the args vector from the Args node, and insert the locals at the end of
            // it.
            auto sorbetArgsNode = translate(up_cast(paramsNode->parameters));
            auto argsNode = parser::cast_node<parser::Args>(sorbetArgsNode.get());
            auto sorbetShadowArgs = translateMulti(paramsNode->locals);
            // Sorbet's legacy parser inserts locals (Shadowargs) at the end of the the block's Args node
            argsNode->args.insert(argsNode->args.end(), make_move_iterator(sorbetShadowArgs.begin()),
                                  make_move_iterator(sorbetShadowArgs.end()));

            return sorbetArgsNode;
        }
        case PM_BREAK_NODE: { // A `break` statement, e.g. `break`, `break 1, 2, 3`
            auto breakNode = down_cast<pm_break_node>(node);

            auto arguments = translateArguments(breakNode->arguments);

            return make_unique<parser::Break>(location, move(arguments));
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
            if (constantNameString == "~" && parser::cast_node<parser::Integer>(receiver.get())) {
                string valueString(sliceLocation(callNode->base.location));

                return make_unique<parser::Integer>(location, move(valueString));
            }

            pm_node_t *prismBlock = callNode->block;
            NodeVec args;
            // PM_BLOCK_ARGUMENT_NODE models the `&b` in `a.map(&b)`,
            // but not an explicit block with `{ ... }` or `do ... end`
            if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE)) {
                args = translateArguments(callNode->arguments, callNode->block);
            } else {
                args = translateArguments(callNode->arguments);
            }

            if (constantNameString == "[]=") {
                messageLoc.endLoc += 2; // The message includes the closing bracket and equals sign
            } else if (constantNameString == "[]") {
                messageLoc.endLoc = messageLoc.beginLoc;
            } else if (constantNameString.back() == '=') {
                messageLoc.endLoc = args.front()->loc.beginPos() - 1; // The message ends right before the equals sign
            }

            unique_ptr<parser::Node> sendNode;
            if (PM_NODE_FLAG_P(callNode, PM_CALL_NODE_FLAGS_SAFE_NAVIGATION)) { // Handle conditional send, e.g. `a&.b`
                sendNode = make_unique<parser::CSend>(loc, move(receiver), gs.enterNameUTF8(constantNameString),
                                                      messageLoc, move(args));
            } else { // Regular send, e.g. `a.b`
                sendNode = make_unique<parser::Send>(loc, move(receiver), gs.enterNameUTF8(constantNameString),
                                                     messageLoc, move(args));
            }

            if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE)) {
                // PM_BLOCK_NODE models an explicit block arg with `{ ... }` or
                // `do ... end`, but not a forwarded block like the `&b` in `a.map(&b)`.
                // In Prism, this is modeled by a `pm_call_node` with a `pm_block_node` as a child, but the
                // The legacy parser inverts this , with a parent "Block" with a child
                // "Send".
                return translateCallWithBlock(prismBlock, move(sendNode));
            } else {
                return sendNode;
            }
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
            auto body = translate(classNode->body);

            if (superclass != nullptr) {
                declLoc = declLoc.join(superclass->loc);
            }

            return make_unique<parser::Class>(location, declLoc, move(name), move(superclass), move(body));
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

            return make_unique<parser::CVar>(location, name);
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

            auto name = translateConstantName(defNode->name);

            // These 2 need to be called on a new Translator with isInMethod set to true
            Translator childContext = enterMethodDef();

            unique_ptr<parser::Node> params;

            // The definition has no parameters but still has parentheses, e.g. `def foo(); end`
            // In this case, Sorbet's legacy parser will still hold an empty Args node, so we need to
            // create one here
            if (defNode->parameters == nullptr && rparenLoc.start != nullptr) {
                params = make_unique<parser::Args>(location, NodeVec{});
            } else {
                params = childContext.translate(up_cast(defNode->parameters));
            }

            auto body = childContext.translate(defNode->body);

            if (defNode->body != nullptr && PM_NODE_TYPE_P(defNode->body, PM_BEGIN_NODE)) {
                // If the body is a PM_BEGIN_NODE instead of a PM_STATEMENTS_NODE, it means the method definition
                // doesn't have an explicit begin block.
                // If that's the case, we need to check if the body contains an ensure or rescue clause, and if so,
                // we need to elevate that node to the top level of the method definition, without the Kwbegin node to
                // match the behavior of Sorbet's legacy parser.
                auto kwbeginNode = parser::cast_node<parser::Kwbegin>(body.get());

                if (kwbeginNode != nullptr && kwbeginNode->stmts[0] != nullptr &&
                    (parser::cast_node<parser::Rescue>(kwbeginNode->stmts[0].get()) ||
                     parser::cast_node<parser::Ensure>(kwbeginNode->stmts[0].get()))) {
                    if (kwbeginNode->stmts.size() == 1) {
                        body = move(kwbeginNode->stmts[0]);
                    } else {
                        unreachable("With ensure or rescue, the body of a method definition will be either a rescue or "
                                    "ensure node.");
                    }
                }
            }

            if (auto receiver = defNode->receiver; receiver != nullptr) {
                auto sorbetReceiver = translate(receiver);
                return make_unique<parser::DefS>(location, declLoc, move(sorbetReceiver), name, move(params),
                                                 move(body));
            }

            return make_unique<parser::DefMethod>(location, declLoc, name, move(params), move(body));
        }
        case PM_DEFINED_NODE: {
            auto definedNode = down_cast<pm_defined_node>(node);

            auto arg = translate(definedNode->value);

            return make_unique<parser::Defined>(location.join(arg->loc), move(arg));
        }
        case PM_ELSE_NODE: { // An `else` clauses, which can pertain to an `if`, `begin`, `case`, etc.
            auto elseNode = down_cast<pm_else_node>(node);
            return translate(up_cast(elseNode->statements));
        }
        case PM_EMBEDDED_STATEMENTS_NODE: { // Statements interpolated into a string.
            // e.g. the `#{bar}` in `"foo #{bar} baz"`
            // Can be multiple statements separated by `;`.
            auto embeddedStmtsNode = down_cast<pm_embedded_statements_node>(node);

            if (auto stmtsNode = embeddedStmtsNode->statements; stmtsNode != nullptr) {
                auto inlineIfSingle = false;
                return translateStatements(stmtsNode, inlineIfSingle);
            } else {
                return make_unique<parser::Begin>(translateLoc(embeddedStmtsNode->base.location), NodeVec{});
            }
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

            return make_unique<parser::Float>(location, move(valueString));
        }
        case PM_FLIP_FLOP_NODE: { // A flip-flop pattern, like the `flip..flop` in `if flip..flop`
            auto flipFlopNode = down_cast<pm_flip_flop_node>(node);

            auto left = patternTranslate(flipFlopNode->left);
            auto right = patternTranslate(flipFlopNode->right);

            if (PM_NODE_FLAG_P(flipFlopNode, PM_RANGE_FLAGS_EXCLUDE_END)) { // 3 dots: `flip...flop`
                return make_unique<parser::EFlipflop>(location, move(left), move(right));
            } else { // 2 dots: `flip..flop`
                return make_unique<parser::IFlipflop>(location, move(left), move(right));
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
            return make_unique<parser::ForwardArg>(location);
        }
        case PM_FORWARDING_SUPER_NODE: { // `super` with no `(...)`
            auto forwardingSuperNode = down_cast<pm_forwarding_super_node>(node);
            auto translatedNode = make_unique<parser::ZSuper>(location);

            auto blockArgumentNode = forwardingSuperNode->block;

            if (blockArgumentNode != nullptr) { // always a PM_BLOCK_NODE
                return translateCallWithBlock(up_cast(blockArgumentNode), move(translatedNode));
            }

            return move(translatedNode);
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

            return make_unique<parser::GVar>(location, name);
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
            return make_unique<parser::Hash>(location, false, move(kvPairs));
        }
        case PM_IF_NODE: { // An `if` statement or modifier, like `if cond; ...; end` or `a.b if cond`
            auto ifNode = down_cast<pm_if_node>(node);

            auto predicate = translate(ifNode->predicate);
            auto ifTrue = translate(up_cast(ifNode->statements));
            auto ifFalse = translate(ifNode->subsequent);

            return make_unique<parser::If>(location, move(predicate), move(ifTrue), move(ifFalse));
        }
        case PM_IMAGINARY_NODE: { // An imaginary number literal, like `1.0i`, `+1.0i`, or `-1.0i`
            auto imaginaryNode = down_cast<pm_imaginary_node>(node);
            // Create a string_view of the value without the trailing 'i'
            auto value = sliceLocation(imaginaryNode->base.location);
            value = value.substr(0, value.size() - 1);

            ENFORCE(!value.empty());

            char sign = value[0];
            // Check for optional leading '+' or '-'
            if (sign == '+' || sign == '-') {
                value.remove_prefix(1);

                // Create the Complex node with the unsigned value
                auto receiver = make_unique<parser::Complex>(location, string(value));

                // Return the appropriate unary operation
                core::NameRef unaryOp = (sign == '-') ? core::Names::unaryMinus() : core::Names::unaryPlus();
                return make_unique<parser::Send>(location, move(receiver), unaryOp,
                                                 core::LocOffsets{location.beginLoc, location.beginLoc + 1}, NodeVec{});
            }

            // No leading sign; return the Complex node directly
            return make_unique<parser::Complex>(location, string(value));
        }
        case PM_IMPLICIT_NODE: { // A hash key without explicit value, like the `k4` in `{ k4: }`
            auto implicitNode = down_cast<pm_implicit_node>(node);
            return translate(implicitNode->value);
        }
        case PM_IMPLICIT_REST_NODE: { // An implicit splat, like the `,` in `a, = 1, 2, 3`
            auto restLoc = core::LocOffsets{location.beginLoc + 1, location.beginLoc + 1};
            return make_unique<parser::Restarg>(restLoc, core::Names::restargs(), restLoc);
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

            return make_unique<parser::IVar>(location, name);
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

            return make_unique<parser::Integer>(location, move(valueString));
        }
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE: { // An interpolated regex literal in a conditional...
            // ...that implicitly checks against the last read line by an IO object, e.g. `if /wat #{123}/`
            auto interpolatedMatchLastLineNode = down_cast<pm_interpolated_match_last_line_node>(node);

            auto parts = translateMulti(interpolatedMatchLastLineNode->parts);
            auto options = translateRegexpOptions(interpolatedMatchLastLineNode->closing_loc);
            auto regex = make_unique<parser::Regexp>(location, move(parts), move(options));

            return make_unique<parser::MatchCurLine>(location, move(regex));
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

            return make_unique<parser::DString>(location, move(sorbetParts));
        }
        case PM_INTERPOLATED_SYMBOL_NODE: { // A symbol like `:"a #{b} c"`
            auto interpolatedSymbolNode = down_cast<pm_interpolated_symbol_node>(node);

            auto sorbetParts = translateMulti(interpolatedSymbolNode->parts);

            return make_unique<parser::DSymbol>(location, move(sorbetParts));
        }
        case PM_INTERPOLATED_X_STRING_NODE: { // An executable string with backticks, like `echo "Hello, world!"`
            auto interpolatedXStringNode = down_cast<pm_interpolated_x_string_node>(node);

            auto sorbetParts = translateMulti(interpolatedXStringNode->parts);

            return make_unique<parser::XString>(location, move(sorbetParts));
        }
        case PM_IT_LOCAL_VARIABLE_READ_NODE: { // The `it` implicit parameter added in Ruby 3.4, e.g. `a.map { it + 1 }`
            [[fallthrough]];
        }
        case PM_IT_PARAMETERS_NODE: { // Used in a parameter list for lambdas that the `it` implicit parameter.
            // See Prism::ParserStorage::ParsedRubyVersion
            unreachable("The `it` keyword was introduced in Ruby 3.4, which isn't supported by Sorbet yet.");
        }
        case PM_KEYWORD_HASH_NODE: { // A hash of keyword arguments, like `foo(a: 1, b: 2)`
            auto kvPairs = translateKeyValuePairs(down_cast<pm_keyword_hash_node>(node)->elements);
            auto isKwargs = absl::c_all_of(kvPairs, [](const auto &node) { return isKeywordHashElement(node.get()); });

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
                sorbetName = gs.freshNameUnique(core::UniqueNameKind::Parser, core::Names::starStar(), nextUniqueID());
            }

            auto kwrestLoc = core::LocOffsets{location.beginPos() + 2, location.endPos()};
            return make_unique<parser::Kwrestarg>(kwrestLoc, sorbetName);
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

            return make_unique<parser::LVar>(location, name);
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // Target of an indirect write to a local variable
            // ... like `target1, target2 = 1, 2`, `rescue => target`, etc.
            auto localVarTargetNode = down_cast<pm_local_variable_target_node>(node);
            auto name = translateConstantName(localVarTargetNode->name);

            return make_unique<parser::LVarLhs>(location, name);
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: { // Regular assignment to a local variable, e.g. `local = 1`
            return translateAssignment<pm_local_variable_write_node, parser::LVarLhs>(node);
        }
        case PM_MATCH_LAST_LINE_NODE: { // A regex literal in a conditional...
            // ...that implicitly checks against the last read line by an IO object, e.g. `if /wat/`
            auto matchLastLineNode = down_cast<pm_match_last_line_node>(node);

            auto regex = translateRegexp(matchLastLineNode->unescaped, location, matchLastLineNode->closing_loc);

            return make_unique<parser::MatchCurLine>(location, move(regex));
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
            auto body = translate(moduleNode->body);

            return make_unique<parser::Module>(location, declLoc, move(name), move(body));
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

            return make_unique<parser::Next>(location, move(arguments));
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
                // The location is arbitrary and not really used, since these aren't explicitly written in the source.
                auto paramNode = make_unique<parser::LVar>(location, gs.enterNameUTF8("_" + to_string(i)));
                params.emplace_back(move(paramNode));
            }

            return make_unique<parser::NumParams>(location, move(params));
        }
        case PM_NUMBERED_REFERENCE_READ_NODE: {
            auto numberedReferenceReadNode = down_cast<pm_numbered_reference_read_node>(node);
            auto number = numberedReferenceReadNode->number;

            return make_unique<parser::NthRef>(location, number);
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: { // An optional keyword parameter, like `def foo(a: 1)`
            auto optionalKeywordParamNode = down_cast<pm_optional_keyword_parameter_node>(node);
            auto nameLoc = translateLoc(optionalKeywordParamNode->name_loc);

            auto name = translateConstantName(optionalKeywordParamNode->name);
            auto value = translate(optionalKeywordParamNode->value);

            return make_unique<parser::Kwoptarg>(location, name, nameLoc, move(value));
        }
        case PM_OPTIONAL_PARAMETER_NODE: { // An optional positional parameter, like `def foo(a = 1)`
            auto optionalParamNode = down_cast<pm_optional_parameter_node>(node);
            auto nameLoc = translateLoc(optionalParamNode->name_loc);

            auto name = translateConstantName(optionalParamNode->name);
            auto value = translate(optionalParamNode->value);

            return make_unique<parser::Optarg>(location, name, nameLoc, move(value));
        }
        case PM_OR_NODE: { // operator `||` and `or`
            auto orNode = down_cast<pm_or_node>(node);

            auto left = translate(orNode->left);
            auto right = translate(orNode->right);

            return make_unique<parser::Or>(location, move(left), move(right));
        }
        case PM_PARAMETERS_NODE: { // The parameters declared at the top of a PM_DEF_NODE
            auto paramsNode = down_cast<pm_parameters_node>(node);

            auto requireds = absl::MakeSpan(paramsNode->requireds.nodes, paramsNode->requireds.size);
            auto optionals = absl::MakeSpan(paramsNode->optionals.nodes, paramsNode->optionals.size);
            auto keywords = absl::MakeSpan(paramsNode->keywords.nodes, paramsNode->keywords.size);
            auto posts = absl::MakeSpan(paramsNode->posts.nodes, paramsNode->posts.size);

            parser::NodeVec params;

            auto restSize = paramsNode->rest == nullptr ? 0 : 1;
            auto kwrestSize = paramsNode->keyword_rest == nullptr ? 0 : 1;
            auto blockSize = paramsNode->block == nullptr ? 0 : 1;

            params.reserve(requireds.size() + optionals.size() + restSize + posts.size() + keywords.size() +
                           kwrestSize + blockSize);

            translateMultiInto(params, requireds);
            translateMultiInto(params, optionals);

            if (paramsNode->rest != nullptr)
                params.emplace_back(translate(paramsNode->rest));

            translateMultiInto(params, posts);
            translateMultiInto(params, keywords);

            if (auto prismKwRestNode = paramsNode->keyword_rest; prismKwRestNode != nullptr) {
                switch (PM_NODE_TYPE(prismKwRestNode)) {
                    case PM_KEYWORD_REST_PARAMETER_NODE: // `**kwargs`
                        [[fallthrough]];
                    case PM_FORWARDING_PARAMETER_NODE: { // `**`
                        params.emplace_back(translate(prismKwRestNode));
                        break;
                    }
                    case PM_NO_KEYWORDS_PARAMETER_NODE: { // `**nil`
                        params.emplace_back(make_unique<parser::Kwnilarg>(translateLoc(prismKwRestNode->location)));
                        break;
                    }
                    default:
                        unreachable("Unexpected keyword_rest node type in Hash pattern.");
                }
            }

            if (paramsNode->block != nullptr)
                params.emplace_back(translate(up_cast(paramsNode->block)));

            return make_unique<parser::Args>(location, move(params));
        }
        case PM_PARENTHESES_NODE: { // A parethesized expression, e.g. `(a)`
            auto parensNode = down_cast<pm_parentheses_node>(node);

            if (auto stmtsNode = parensNode->body; stmtsNode != nullptr) {
                auto inlineIfSingle = false;
                return translateStatements(down_cast<pm_statements_node>(stmtsNode), inlineIfSingle);
            } else {
                return make_unique<parser::Begin>(location, NodeVec{});
            }
        }
        case PM_PRE_EXECUTION_NODE: {
            auto preExecutionNode = down_cast<pm_pre_execution_node>(node);
            auto body = translateStatements(preExecutionNode->statements);
            return make_unique<parser::Preexe>(location, move(body));
        }
        case PM_PROGRAM_NODE: { // The root node of the parse tree, representing the entire program
            pm_program_node *programNode = down_cast<pm_program_node>(node);

            return translate(up_cast(programNode->statements));
        }
        case PM_POST_EXECUTION_NODE: {
            auto postExecutionNode = down_cast<pm_post_execution_node>(node);
            auto body = translateStatements(postExecutionNode->statements);
            return make_unique<parser::Postexe>(location, move(body));
        }
        case PM_RANGE_NODE: { // A Range literal, e.g. `a..b`, `a..`, `..b`, `a...b`, `a...`, `...b`
            auto rangeNode = down_cast<pm_range_node>(node);

            auto left = translate(rangeNode->left);
            auto right = translate(rangeNode->right);

            if (PM_NODE_FLAG_P(rangeNode, PM_RANGE_FLAGS_EXCLUDE_END)) { // `...`
                return make_unique<parser::ERange>(location, move(left), move(right));
            } else { // `..`
                return make_unique<parser::IRange>(location, move(left), move(right));
            }
        }
        case PM_RATIONAL_NODE: { // A rational number literal, e.g. `1r`
            // Note: in `1/2r`, only the `2r` is part of the `PM_RATIONAL_NODE`.
            // The `1/` is just divison of an integer.
            auto *rationalNode = down_cast<pm_rational_node>(node);

            // `-1` drops the trailing `r` end of the value
            auto value = sliceLocation(rationalNode->base.location);
            value = value.substr(0, value.size() - 1);

            return make_unique<parser::Rational>(location, value);
        }
        case PM_REDO_NODE: { // The `redo` keyword
            return make_unique<parser::Redo>(location);
        }
        case PM_REGULAR_EXPRESSION_NODE: { // A regular expression literal, e.g. `/foo/`
            auto regexNode = down_cast<pm_regular_expression_node>(node);

            return translateRegexp(regexNode->unescaped, location, regexNode->closing_loc);
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: { // A required keyword parameter, like `def foo(a:)`
            auto requiredKeywordParamNode = down_cast<pm_required_keyword_parameter_node>(node);
            auto name = translateConstantName(requiredKeywordParamNode->name);

            return make_unique<parser::Kwarg>(location, name);
        }
        case PM_REQUIRED_PARAMETER_NODE: { // A required positional parameter, like `def foo(a)`
            auto requiredParamNode = down_cast<pm_required_parameter_node>(node);
            auto name = translateConstantName(requiredParamNode->name);

            return make_unique<parser::Arg>(location, name);
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

            return make_unique<parser::Restarg>(location, sorbetName, nameLoc);
        }
        case PM_RETURN_NODE: { // A `return` statement, like `return 1, 2, 3`
            auto returnNode = down_cast<pm_return_node>(node);

            auto returnValues = translateArguments(returnNode->arguments);

            return make_unique<parser::Return>(location, move(returnValues));
        }
        case PM_RETRY_NODE: { // The `retry` keyword
            return make_unique<parser::Retry>(location);
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
            pm_location_t declLoc = classNode->class_keyword_loc;

            auto expr = translate(classNode->expression);
            auto body = translate(classNode->body);

            return make_unique<parser::SClass>(location, translateLoc(declLoc), move(expr), move(body));
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
            return make_unique<parser::LineLiteral>(location);
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
            auto source = parser.extractString(unescaped);

            // TODO: handle different string encodings
            return make_unique<parser::String>(location, gs.enterNameUTF8(source));
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

            auto unescaped = &symNode->unescaped;

            auto source = parser.extractString(unescaped);

            // If the opening location is null, the symbol is used as a key with a colon postfix, like `{a: 1}`
            // In those cases, the location should not include the colon.
            if (symNode->opening_loc.start == nullptr) {
                location = translateLoc(symNode->value_loc);
            }

            // TODO: can these have different encodings?
            return make_unique<parser::Symbol>(location, gs.enterNameUTF8(source));
        }
        case PM_TRUE_NODE: { // The `true` keyword
            return make_node_with_expr<parser::True>(MK::True(location), location);
        }
        case PM_UNDEF_NODE: { // The `undef` keyword, like `undef :method_to_undef
            auto undefNode = down_cast<pm_undef_node>(node);

            auto names = translateMulti(undefNode->names);

            return make_unique<parser::Undef>(location, move(names));
        }
        case PM_UNLESS_NODE: { // An `unless` branch, either in a statement or modifier form.
            auto unlessNode = down_cast<pm_unless_node>(node);

            auto predicate = translate(unlessNode->predicate);
            // These are flipped relative to `PM_IF_NODE`
            auto ifFalse = translate(up_cast(unlessNode->statements));
            auto ifTrue = translate(up_cast(unlessNode->else_clause));

            return make_unique<parser::If>(location, move(predicate), move(ifTrue), move(ifFalse));
        }
        case PM_UNTIL_NODE: { // A `until` loop, like `until stop_condition; ...; end`
            auto untilNode = down_cast<pm_until_node>(node);

            auto predicate = translate(untilNode->predicate);
            auto body = translate(up_cast(untilNode->statements));

            // When the until loop is placed after a `begin` block, like `begin; end until false`,
            if (PM_NODE_FLAG_P(untilNode, PM_LOOP_FLAGS_BEGIN_MODIFIER)) {
                return make_unique<parser::UntilPost>(location, move(predicate), move(body));
            } else {
                return make_unique<parser::Until>(location, move(predicate), move(body));
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
            auto statements = translateStatements(whileNode->statements);

            // When the while loop is placed after a `begin` block, like `begin; end while false`,
            if (PM_NODE_FLAG_P(whileNode, PM_LOOP_FLAGS_BEGIN_MODIFIER)) {
                return make_unique<parser::WhilePost>(location, move(predicate), move(statements));
            } else {
                return make_unique<parser::While>(location, move(predicate), move(statements));
            }
        }
        case PM_X_STRING_NODE: { // An interpolated x-string, like `/usr/bin/env ls`
            auto strNode = down_cast<pm_x_string_node>(node);

            auto unescaped = &strNode->unescaped;
            auto source = parser.extractString(unescaped);

            // TODO: handle different string encodings
            unique_ptr<parser::Node> string = make_unique<parser::String>(location, gs.enterNameUTF8(source));

            NodeVec nodes{};
            nodes.emplace_back(move(string)); // Multiple nodes is only possible for interpolated x strings.

            return make_unique<parser::XString>(location, move(nodes));
        }
        case PM_YIELD_NODE: { // The `yield` keyword, like `yield`, `yield 1, 2, 3`
            auto yieldNode = down_cast<pm_yield_node>(node);

            auto yieldArgs = translateArguments(yieldNode->arguments);

            return make_unique<parser::Yield>(location, move(yieldArgs));
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

core::LocOffsets Translator::translateLoc(pm_location_t loc) {
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

            if (auto prismConstant = arrayPatternNode->constant; prismConstant != nullptr) {
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

            if (auto prismConstant = hashPatternNode->constant; prismConstant != nullptr) {
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
            auto beginNode = make_unique<parser::Begin>(translateLoc(pinnedExprNode->base.location), move(statements));

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
            auto splatLoc = translateLoc(prismSplatNode->base.location);
            return make_unique<MatchRest>(splatLoc, move(expr));
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

// Copied from `Builder::isKeywordHashElement()`
// Checks if the given node is a keyword hash element based on the standards of Sorbet's legacy parser.
bool Translator::isKeywordHashElement(sorbet::parser::Node *node) {
    if (NodeWithExpr::isa_node<Kwsplat>(node)) {
        return true;
    }

    if (NodeWithExpr::isa_node<ForwardedKwrestArg>(node)) {
        return true;
    }

    if (auto *pair = NodeWithExpr::cast_node<Pair>(node)) {
        return NodeWithExpr::isa_node<Symbol>(pair->key.get());
    }

    return false;
}

// Prism models a call with an explicit block argument as a `pm_call_node` that contains a `pm_block_node`.
// Sorbet's legacy parser models this the other way around, as a parent `Block` with a child `Send`.
// Lambda literals also have a similar reverse structure between the 2 parsers.
//
// This function translates between the two, creating a `Block` or `NumBlock` node for the given `pm_block_node *`
// or `pm_lambda_node *`, and wrapping it around the given `Send` node.
unique_ptr<parser::Node> Translator::translateCallWithBlock(pm_node_t *prismBlockOrLambdaNode,
                                                            unique_ptr<parser::Node> sendNode) {
    unique_ptr<parser::Node> parametersNode;
    unique_ptr<parser::Node> body;

    if (PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_BLOCK_NODE)) {
        auto prismBlockNode = down_cast<pm_block_node>(prismBlockOrLambdaNode);
        parametersNode = translate(prismBlockNode->parameters);
        body = translate(prismBlockNode->body);
    } else {
        ENFORCE(PM_NODE_TYPE_P(prismBlockOrLambdaNode, PM_LAMBDA_NODE))
        auto prismLambdaNode = down_cast<pm_lambda_node>(prismBlockOrLambdaNode);
        parametersNode = translate(prismLambdaNode->parameters);
        body = translate(prismLambdaNode->body);
    }

    if (parser::cast_node<parser::NumParams>(parametersNode.get())) {
        return make_unique<parser::NumBlock>(sendNode->loc, move(sendNode), move(parametersNode), move(body));
    } else {
        return make_unique<parser::Block>(sendNode->loc, move(sendNode), move(parametersNode), move(body));
    }
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

        rescueBodies.emplace_back(make_unique<parser::Resbody>(translateLoc(currentRescueNode->base.location),
                                                               move(exceptionsArray), move(var), move(rescueBody)));
    }

    // The `Rescue` node combines the main body, the rescue clauses, and the else clause.
    return make_unique<parser::Rescue>(rescueLoc, move(bodyNode), move(rescueBodies), move(elseNode));
}

// Translates the given Prism Statements Node into a `parser::Begin` node or an inlined `parser::Node`.
// @param inlineIfSingle If enabled and there's 1 child node, we skip the `Begin` and just return the one `parser::Node`
unique_ptr<parser::Node> Translator::translateStatements(pm_statements_node *stmtsNode, bool inlineIfSingle) {
    if (stmtsNode == nullptr)
        return nullptr;

    // For a single statement, do not create a `Begin` node and just return the statement, if that's enabled.
    if (inlineIfSingle && stmtsNode->body.size == 1) {
        return translate(stmtsNode->body.nodes[0]);
    }

    // For multiple statements, convert each statement and add them to the body of a Begin node
    parser::NodeVec sorbetStmts = translateMulti(stmtsNode->body);

    return make_unique<parser::Begin>(translateLoc(stmtsNode->base.location), move(sorbetStmts));
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

    if (isInMethodDef && replaceWithDynamicConstAssign) {
        // Check if this is a dynamic constant assignment (SyntaxError at runtime)
        // This is a copy of a workaround from `Desugar.cc`, which substitues in a fake assignment,
        // so the parsing can continue. See other usages of `dynamicConstAssign` for more details.
        return make_unique<LVarLhs>(location, core::Names::dynamicConstAssign());
    }

    auto constexpr isConstantPath = is_same_v<PrismLhsNode, pm_constant_path_target_node> ||
                                    is_same_v<PrismLhsNode, pm_constant_path_write_node> ||
                                    is_same_v<PrismLhsNode, pm_constant_path_node>;

    unique_ptr<parser::Node> parent;
    if constexpr (isConstantPath) { // Handle constant paths, has a parent node that needs translation.
        if (auto prismParentNode = node->parent; prismParentNode != nullptr) {
            // This constant reference is chained onto another constant reference.
            // E.g. given `A::B::C`, if `node` is pointing to the root, `A::B` is the `parent`, and `C` is the `name`.
            //   A::B::C
            //    /    \
            //  A::B   ::C
            //  /  \
            // A   ::B
            parent = translate(prismParentNode);
        } else { // This is the root of a fully qualified constant reference, like `::A`.
            auto delimiterLoc = translateLoc(node->delimiter_loc); // The location of the `::`
            parent = make_unique<parser::Cbase>(delimiterLoc);
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
    }

    return make_unique<SorbetLHSNode>(location, move(parent), gs.enterNameConstant(name));
}

core::NameRef Translator::translateConstantName(pm_constant_id_t constantId) {
    return gs.enterNameUTF8(parser.resolveConstant(constantId));
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
        auto sourceStringNode = make_unique<parser::String>(location, gs.enterNameUTF8(source));
        parts.emplace_back(move(sourceStringNode));
    }

    auto options = translateRegexpOptions(closingLoc);

    return make_unique<parser::Regexp>(location, move(parts), move(options));
}

string_view Translator::sliceLocation(pm_location_t loc) {
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
                        make_unique<parser::Restarg>(location, name, translateLoc(requiredParamNode->base.location)));
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

// Context management methods
Translator Translator::enterMethodDef() {
    auto isInMethodDef = true;
    return Translator(parser, gs, file, parseErrors, isInMethodDef, uniqueCounter);
}

void Translator::reportError(core::LocOffsets loc, const string &message) {
    auto errorLoc = core::Loc(file, loc);
    if (auto e = gs.beginError(errorLoc, core::errors::Parser::ParserError)) {
        e.setHeader("{}", message);
    }
}
}; // namespace sorbet::parser::Prism
