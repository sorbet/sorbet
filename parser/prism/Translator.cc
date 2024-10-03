#include "Translator.h"

template class std::unique_ptr<sorbet::parser::Node>;

using std::make_unique;
using std::unique_ptr;

namespace sorbet::parser::Prism {

// Indicates that a particular code path should never be reached, with an explanation of why.
// Throws a `sorbet::SorbetException` in debug mode, and is undefined behaviour otherwise.
template <typename... TArgs>
[[noreturn]] void unreachable(fmt::format_string<TArgs...> reason_format_str, TArgs &&...args) {
    if constexpr (sorbet::debug_mode) {
        Exception::raise(reason_format_str, std::forward<TArgs>(args)...);
    } else {
        // Basically a backport of C++23's `std::unreachable()`:
        // > `ABSL_UNREACHABLE()` is an unreachable statement.  A program which reaches
        // > one has undefined behavior, and the compiler may optimize accordingly.
        ABSL_UNREACHABLE();
    }
}

template <typename PrismAssignmentNode, typename SorbetLHSNode>
std::unique_ptr<parser::Assign> Translator::translateAssignment(pm_node_t *untypedNode) {
    auto node = reinterpret_cast<PrismAssignmentNode *>(untypedNode);
    auto *loc = &node->base.location;

    auto rhs = translate(node->value);
    unique_ptr<parser::Node> lhs;

    if constexpr (std::is_same_v<PrismAssignmentNode, pm_constant_write_node>) {
        // Handle regular assignment to a "plain" constant, like `A = 1`
        lhs = translateConst<pm_constant_write_node, parser::ConstLhs>(node);
    } else if constexpr (std::is_same_v<PrismAssignmentNode, pm_constant_path_write_node>) {
        // Handle regular assignment to a constant path, like `A::B::C = 1` or `::C = 1`
        lhs = translateConst<pm_constant_path_node, parser::ConstLhs>(node->target);
    } else {
        // Handle regular assignment to any other kind of LHS.
        auto name = parser.resolveConstant(node->name);
        lhs = make_unique<SorbetLHSNode>(parser.translateLocation(&node->name_loc), gs.enterNameUTF8(name));
    }

    return make_unique<parser::Assign>(parser.translateLocation(loc), std::move(lhs), std::move(rhs));
}

template <typename PrismAssignmentNode, typename SorbetAssignmentNode, typename SorbetLHSNode>
std::unique_ptr<SorbetAssignmentNode> Translator::translateOpAssignment(pm_node_t *untypedNode) {
    static_assert(
        std::is_same_v<SorbetAssignmentNode, parser::OpAsgn> || std::is_same_v<SorbetAssignmentNode, parser::AndAsgn> ||
            std::is_same_v<SorbetAssignmentNode, parser::OrAsgn>,
        "Invalid operator node type. Must be one of `parser::OpAssign`, `parser::AndAsgn` or `parser::OrAsgn`.");

    auto node = reinterpret_cast<PrismAssignmentNode *>(untypedNode);
    auto *loc = &node->base.location;

    unique_ptr<parser::Node> lhs;
    auto rhs = translate(node->value);

    // Various node types need special handling to construct their corresponding Sorbet LHS nodes.
    if constexpr (std::is_same_v<PrismAssignmentNode, pm_index_operator_write_node> ||
                  std::is_same_v<PrismAssignmentNode, pm_index_and_write_node> ||
                  std::is_same_v<PrismAssignmentNode, pm_index_or_write_node>) {
        // Handle operator assignment to an indexed expression, like `a[0] += 1`
        auto *openingLoc = &node->opening_loc;
        auto receiver = translate(node->receiver);
        auto args = translateArguments(node->arguments);
        lhs =
            make_unique<parser::Send>(parser.translateLocation(loc), std::move(receiver), core::Names::squareBrackets(),
                                      parser.translateLocation(openingLoc), std::move(args));
    } else if constexpr (std::is_same_v<PrismAssignmentNode, pm_constant_operator_write_node> ||
                         std::is_same_v<PrismAssignmentNode, pm_constant_and_write_node> ||
                         std::is_same_v<PrismAssignmentNode, pm_constant_or_write_node>) {
        // Handle operator assignment to a "plain" constant, like `A += 1`
        lhs = translateConst<PrismAssignmentNode, parser::ConstLhs>(node);
    } else if constexpr (std::is_same_v<PrismAssignmentNode, pm_constant_path_operator_write_node> ||
                         std::is_same_v<PrismAssignmentNode, pm_constant_path_and_write_node> ||
                         std::is_same_v<PrismAssignmentNode, pm_constant_path_or_write_node>) {
        // Handle operator assignment to a constant path, like `A::B::C += 1` or `::C += 1`
        lhs = translateConst<pm_constant_path_node, parser::ConstLhs>(node->target);
    } else if constexpr (std::is_same_v<SorbetLHSNode, parser::Send>) {
        // Handle operator assignment to the result of a method call, like `a.b += 1`
        auto name = parser.resolveConstant(node->read_name);
        auto receiver = translate(node->receiver);
        auto *message_loc = &node->message_loc;
        lhs = make_unique<parser::Send>(parser.translateLocation(loc), std::move(receiver), gs.enterNameUTF8(name),
                                        parser.translateLocation(message_loc), NodeVec{});
    } else {
        // Handle regular assignment to any other kind of LHS.
        auto *nameLoc = &node->name_loc;
        auto name = parser.resolveConstant(node->name);
        lhs = make_unique<SorbetLHSNode>(parser.translateLocation(nameLoc), gs.enterNameUTF8(name));
    }

    if constexpr (std::is_same_v<SorbetAssignmentNode, parser::OpAsgn>) {
        // `OpAsgn` assign needs more information about the specific operator here, so it gets special handling here.
        auto *opLoc = &node->binary_operator_loc;
        auto op = parser.resolveConstant(node->binary_operator);

        return make_unique<parser::OpAsgn>(parser.translateLocation(loc), std::move(lhs), gs.enterNameUTF8(op),
                                           parser.translateLocation(opLoc), std::move(rhs));
    } else {
        // `AndAsgn` and `OrAsgn` are specific to a single operator, so don't need any extra information like `OpAsgn`.
        static_assert(std::is_same_v<SorbetAssignmentNode, parser::AndAsgn> ||
                      std::is_same_v<SorbetAssignmentNode, parser::OrAsgn>);

        return make_unique<SorbetAssignmentNode>(parser.translateLocation(loc), std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<parser::Node> Translator::translate(pm_node_t *node) {
    if (node == nullptr)
        return nullptr;

    switch (PM_NODE_TYPE(node)) {
        case PM_AND_NODE: { // operator `&&` and `and`
            auto andNode = reinterpret_cast<pm_and_node *>(node);
            auto *loc = &andNode->base.location;

            auto left = translate(andNode->left);
            auto right = translate(andNode->right);

            return make_unique<parser::And>(parser.translateLocation(loc), std::move(left), std::move(right));
        }
        case PM_ARGUMENTS_NODE: { // A list of arguments in one of several places:
            // 1. The arguments to a method call, e.g the `1, 2, 3` in `f(1, 2, 3)`.
            // 2. The value(s) returned from a return statement, e.g. the `1, 2, 3` in `return 1, 2, 3`.
            // 3. The arguments to a `yield` call, e.g. the `1, 2, 3` in `yield 1, 2, 3`.
            unreachable("PM_ARGUMENTS_NODE is handled separately in `Translator::translateArguments()`.");
        }
        case PM_ARRAY_NODE: { // An array literal, e.g. `[1, 2, 3]`
            auto arrayNode = reinterpret_cast<pm_array_node *>(node);
            pm_location_t *loc = &arrayNode->base.location;

            parser::NodeVec sorbetElements = translateMulti(arrayNode->elements);

            return make_unique<parser::Array>(parser.translateLocation(loc), std::move(sorbetElements));
        }
        case PM_ARRAY_PATTERN_NODE: { // An array pattern such as the `[head, *tail]` in the `a in [head, *tail]`
            auto arrayPatternNode = reinterpret_cast<pm_array_pattern_node *>(node);
            pm_location_t *loc = &arrayPatternNode->base.location;

            auto prismPrefixNodes = absl::MakeSpan(arrayPatternNode->requireds.nodes, arrayPatternNode->requireds.size);
            auto prismSplatNode = reinterpret_cast<pm_splat_node *>(arrayPatternNode->rest);
            auto prismSuffixNodes = absl::MakeSpan(arrayPatternNode->posts.nodes, arrayPatternNode->posts.size);

            NodeVec sorbetElements{};
            sorbetElements.reserve(prismPrefixNodes.size() + (prismSplatNode != nullptr ? 1 : 0) +
                                   prismSuffixNodes.size());

            translateMultiInto(sorbetElements, prismPrefixNodes);

            if (prismSplatNode != nullptr) {
                auto expr = translate(prismSplatNode->expression);
                auto splatLoc = &prismSplatNode->base.location;
                sorbetElements.emplace_back(
                    make_unique<MatchRest>(parser.translateLocation(splatLoc), std::move(expr)));
            }

            translateMultiInto(sorbetElements, prismSuffixNodes);

            return make_unique<parser::ArrayPattern>(parser.translateLocation(loc), std::move(sorbetElements));
        }
        case PM_ASSOC_NODE: { // A key-value pair in a Hash literal, e.g. the `a: 1` in `{ a: 1 }
            auto assocNode = reinterpret_cast<pm_assoc_node *>(node);
            pm_location_t *loc = &assocNode->base.location;

            auto key = translate(assocNode->key);
            auto value = translate(assocNode->value);

            return make_unique<parser::Pair>(parser.translateLocation(loc), std::move(key), std::move(value));
        }
        case PM_ASSOC_SPLAT_NODE: { // A Hash splat, e.g. `**h` in `f(a: 1, **h)` and `{ k: v, **h }`
            unreachable("PM_ASSOC_SPLAT_NODE is handled separately in `Translator::translateHash()` and "
                        "`PM_HASH_PATTERN_NODE`, because its translation depends on whether its used in a "
                        "Hash literal, Hash pattern, or method call.");
        }
        case PM_BEGIN_NODE: { // A `begin ... end` block
            auto beginNode = reinterpret_cast<pm_begin_node *>(node);
            auto loc = &beginNode->base.location;

            NodeVec statements;

            if (auto prismStatements = beginNode->statements; prismStatements != nullptr) {
                statements = translateMulti(prismStatements->body);
            }

            return make_unique<parser::Kwbegin>(parser.translateLocation(loc), std::move(statements));
        }
        case PM_BLOCK_ARGUMENT_NODE: { // A block arg passed into a method call, e.g. the `&b` in `a.map(&b)`
            auto blockArg = reinterpret_cast<pm_block_argument_node *>(node);
            auto loc = &blockArg->base.location;

            auto expr = translate(blockArg->expression);

            return make_unique<parser::BlockPass>(parser.translateLocation(loc), std::move(expr));
        }
        case PM_BLOCK_NODE: { // An explicit block passed to a method call, i.e. `{ ... }` or `do ... end
            unreachable("PM_BLOCK_NODE has special handling in translateCallWithBlock, see its docs for details.");
        }
        case PM_BLOCK_PARAMETER_NODE: { // A block parameter declared at the top of a method, e.g. `def m(&block)`
            auto blockParamNode = reinterpret_cast<pm_block_parameter_node *>(node);
            pm_location_t *loc = &blockParamNode->base.location;

            core::NameRef sorbetName;
            if (auto prismName = blockParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named block parameter, like `def foo(&block)`
                auto name = parser.resolveConstant(prismName);
                sorbetName = gs.enterNameUTF8(name);
            } else { // An anonymous block parameter, like `def foo(&)`
                sorbetName =
                    gs.freshNameUnique(core::UniqueNameKind::Parser, core::Names::ampersand(), ++uniqueCounter);
            }

            return make_unique<parser::Blockarg>(parser.translateLocation(loc), sorbetName);
        }
        case PM_BLOCK_PARAMETERS_NODE: { // The parameters declared at the top of a PM_BLOCK_NODE
            auto paramsNode = reinterpret_cast<pm_block_parameters_node *>(node);
            return translate(reinterpret_cast<pm_node *>(paramsNode->parameters));
        }
        case PM_BREAK_NODE: { // A `break` statement, e.g. `break`, `break 1, 2, 3`
            auto breakNode = reinterpret_cast<pm_break_node *>(node);
            pm_location_t *loc = &breakNode->base.location;

            auto arguments = translateArguments(breakNode->arguments);

            return make_unique<parser::Break>(parser.translateLocation(loc), std::move(arguments));
        }
        case PM_CALL_AND_WRITE_NODE: { // And-assignment to a method call, e.g. `a.b &&= false`
            return translateOpAssignment<pm_call_and_write_node, parser::AndAsgn, parser::Send>(node);
        }
        case PM_CALL_NODE: { // A method call like `a.b()` or `a&.b()`
            auto callNode = reinterpret_cast<pm_call_node *>(node);
            pm_location_t *loc = &callNode->base.location;
            pm_location_t *messageLoc = &callNode->message_loc;

            auto name = parser.resolveConstant(callNode->name);
            auto receiver = translate(callNode->receiver);

            pm_node_t *prismBlock = callNode->block;
            // PM_BLOCK_ARGUMENT_NODE models the `&b` in `a.map(&b)`,
            // but not an explicit block with `{ ... }` or `do ... end`
            auto hasBlockArgument = prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_ARGUMENT_NODE);

            parser::NodeVec args = translateArguments(callNode->arguments, (hasBlockArgument ? 0 : 1));

            if (hasBlockArgument) {
                auto blockPassNode = translate(prismBlock);
                args.emplace_back(std::move(blockPassNode));
            }

            auto flags = static_cast<pm_call_node_flags>(callNode->base.flags);

            unique_ptr<parser::Node> sendNode;

            if (flags & PM_CALL_NODE_FLAGS_SAFE_NAVIGATION) { // Handle conditional send, e.g. `a&.b`
                sendNode = make_unique<parser::CSend>(parser.translateLocation(loc), std::move(receiver),
                                                      gs.enterNameUTF8(name), parser.translateLocation(messageLoc),
                                                      std::move(args));
            } else { // Regular send, e.g. `a.b`
                sendNode = make_unique<parser::Send>(parser.translateLocation(loc), std::move(receiver),
                                                     gs.enterNameUTF8(name), parser.translateLocation(messageLoc),
                                                     std::move(args));
            }

            if (prismBlock != nullptr && PM_NODE_TYPE_P(prismBlock, PM_BLOCK_NODE)) {
                // PM_BLOCK_NODE models an explicit block arg with `{ ... }` or
                // `do ... end`, but not a forwarded block like the `&b` in `a.map(&b)`.
                // In Prism, this is modeled by a `pm_call_node` with a `pm_block_node` as a child, but the
                // The legacy parser inverts this , with a parent "Block" with a child
                // "Send".
                return translateCallWithBlock(reinterpret_cast<pm_block_node *>(prismBlock), std::move(sendNode));
            } else {
                return sendNode;
            }
        }
        case PM_CALL_OPERATOR_WRITE_NODE: { // Compound assignment to a method call, e.g. `a.b += 1`
            return translateOpAssignment<pm_call_operator_write_node, parser::OpAsgn, parser::Send>(node);
        }
        case PM_CALL_OR_WRITE_NODE: { // Or-assignment to a method call, e.g. `a.b ||= true`
            return translateOpAssignment<pm_call_or_write_node, parser::OrAsgn, parser::Send>(node);
        }
        case PM_CASE_MATCH_NODE: { // A pattern-matching `case` statement that only uses `in` (and not `when`)
            auto caseMatchNode = reinterpret_cast<pm_case_match_node *>(node);
            pm_location_t *loc = &caseMatchNode->base.location;

            auto predicate = translate(caseMatchNode->predicate);
            auto sorbetConditions = translateMulti(caseMatchNode->conditions);
            auto consequent = translate(reinterpret_cast<pm_node_t *>(caseMatchNode->consequent));

            return make_unique<parser::CaseMatch>(parser.translateLocation(loc), std::move(predicate),
                                                  std::move(sorbetConditions), std::move(consequent));
        }
        case PM_CASE_NODE: { // A classic `case` statement that only uses `when` (and not pattern matching with `in`)
            auto caseNode = reinterpret_cast<pm_case_node *>(node);
            pm_location_t *loc = &caseNode->base.location;

            auto predicate = translate(caseNode->predicate);
            auto sorbetConditions = translateMulti(caseNode->conditions);
            auto consequent = translate(reinterpret_cast<pm_node_t *>(caseNode->consequent));

            return make_unique<Case>(parser.translateLocation(loc), std::move(predicate), std::move(sorbetConditions),
                                     std::move(consequent));
        }
        case PM_CLASS_NODE: { // Class declarations, not including singleton class declarations (`class <<`)
            auto classNode = reinterpret_cast<pm_class_node *>(node);
            pm_location_t *loc = &classNode->base.location;
            pm_location_t *declLoc = &classNode->class_keyword_loc;

            auto name = translate(classNode->constant_path);
            auto superclass = translate(classNode->superclass);

            auto body = translate(classNode->body);

            return make_unique<parser::Class>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                              std::move(name), std::move(superclass), std::move(body));
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
            auto classVarNode = reinterpret_cast<pm_class_variable_read_node *>(node);
            pm_location_t *loc = &classVarNode->base.location;

            std::string_view name = parser.resolveConstant(classVarNode->name);

            return make_unique<parser::CVar>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_CLASS_VARIABLE_WRITE_NODE: { // Regular assignment to a class variable, e.g. `@@a = 1`
            return translateAssignment<pm_class_variable_write_node, parser::CVarLhs>(node);
        }
        case PM_CONSTANT_PATH_AND_WRITE_NODE: { // And-assignment to a constant path, e.g. `A::B &&= false`
            return translateOpAssignment<pm_constant_path_and_write_node, parser::AndAsgn, parser::ConstLhs>(node);
        }
        case PM_CONSTANT_PATH_NODE: { // Part of a constant path, like the `A::B` in `A::B::C`.
            // See`PM_CONSTANT_READ_NODE`, which handles the `::C` part
            auto constantPathNode = reinterpret_cast<pm_constant_path_node *>(node);

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
            auto constantPathTargetNode = reinterpret_cast<pm_constant_path_target_node *>(node);

            return translateConst<pm_constant_path_target_node, parser::ConstLhs>(constantPathTargetNode);
        }
        case PM_CONSTANT_PATH_WRITE_NODE: { // Regular assignment to a constant path, e.g. `A::B = 1`
            return translateAssignment<pm_constant_path_write_node, void>(node);
        }
        case PM_CONSTANT_TARGET_NODE: { // Target of an indirect write to a constant
            // ... like `TARGET1, TARGET2 = 1, 2`, `rescue => TARGET`, etc.
            auto constantTargetNode = reinterpret_cast<pm_constant_target_node *>(node);
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
            auto constantReadNode = reinterpret_cast<pm_constant_read_node *>(node);
            return translateConst<pm_constant_read_node, parser::Const>(constantReadNode);
        }
        case PM_CONSTANT_WRITE_NODE: { // Regular assignment to a constant, e.g. `Foo = 1`
            return translateAssignment<pm_constant_write_node, parser::ConstLhs>(node);
        }
        case PM_DEF_NODE: { // Method definitions, like `def m; ...; end` and `def m = 123`
            auto defNode = reinterpret_cast<pm_def_node *>(node);
            pm_location_t *loc = &defNode->base.location;
            pm_location_t *declLoc = &defNode->def_keyword_loc;

            std::string_view name = parser.resolveConstant(defNode->name);
            auto params = translate(reinterpret_cast<pm_node *>(defNode->parameters));
            auto body = translate(defNode->body);

            return make_unique<parser::DefMethod>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                                  gs.enterNameUTF8(name), std::move(params), std::move(body));
        }
        case PM_ELSE_NODE: { // An `else` clauses, which can pertain to an `if`, `begin`, `case`, etc.
            auto elseNode = reinterpret_cast<pm_else_node *>(node);
            return translate(reinterpret_cast<pm_node *>(elseNode->statements));
        }
        case PM_EMBEDDED_STATEMENTS_NODE: { // Statements interpolated into a string.
            // e.g. the `#{bar}` in `"foo #{bar} baz"`
            // Can be multiple statements separated by `;`.

            auto embeddedStmtsNode = reinterpret_cast<pm_embedded_statements_node *>(node);

            if (auto stmtsNode = embeddedStmtsNode->statements; stmtsNode != nullptr) {
                auto inlineIfSingle = false;
                return translateStatements(stmtsNode, inlineIfSingle);
            } else {
                return make_unique<parser::Begin>(parser.translateLocation(&embeddedStmtsNode->base.location),
                                                  NodeVec{});
            }
        }
        case PM_FALSE_NODE: { // The `false` keyword
            return translateSimpleKeyword<pm_false_node, parser::False>(node);
        }
        case PM_FIND_PATTERN_NODE: { // A find pattern such as the `[*, middle, *]` in the `a in [*, middle, *]`
            auto findPatternNode = reinterpret_cast<pm_find_pattern_node *>(node);
            pm_location_t *loc = &findPatternNode->base.location;

            auto prismLeadingSplat = findPatternNode->left;
            auto prismMiddleNodes = absl::MakeSpan(findPatternNode->requireds.nodes, findPatternNode->requireds.size);
            auto prismTrailingSplat = findPatternNode->right;

            NodeVec sorbetElements{};
            sorbetElements.reserve(1 + prismMiddleNodes.size() + (prismTrailingSplat != nullptr ? 1 : 0));

            if (prismLeadingSplat != nullptr && PM_NODE_TYPE_P(prismLeadingSplat, PM_SPLAT_NODE)) {
                auto prismSplatNode = reinterpret_cast<pm_splat_node *>(prismLeadingSplat);
                auto expr = translate(prismSplatNode->expression);
                auto splatLoc = &prismSplatNode->base.location;
                sorbetElements.emplace_back(
                    make_unique<MatchRest>(parser.translateLocation(splatLoc), std::move(expr)));
            }

            translateMultiInto(sorbetElements, prismMiddleNodes);

            if (prismTrailingSplat != nullptr && PM_NODE_TYPE_P(prismTrailingSplat, PM_SPLAT_NODE)) {
                // TODO: handle PM_NODE_TYPE_P(prismTrailingSplat, PM_MISSING_NODE)
                auto prismSplatNode = reinterpret_cast<pm_splat_node *>(prismTrailingSplat);
                auto expr = translate(prismSplatNode->expression);
                auto splatLoc = &prismSplatNode->base.location;
                sorbetElements.emplace_back(
                    make_unique<MatchRest>(parser.translateLocation(splatLoc), std::move(expr)));
            }

            return make_unique<parser::FindPattern>(parser.translateLocation(loc), std::move(sorbetElements));
        }
        case PM_FLOAT_NODE: { // A floating point number literal, e.g. `1.23`
            auto floatNode = reinterpret_cast<pm_float_node *>(node);
            pm_location_t *loc = &floatNode->base.location;

            return make_unique<parser::Float>(parser.translateLocation(loc), std::to_string(floatNode->value));
        }
        case PM_FORWARDING_ARGUMENTS_NODE: { // The `...` argument in a method call, like `foo(...)`
            return translateSimpleKeyword<pm_forwarding_arguments_node, parser::ForwardedArgs>(node);
        }
        case PM_FORWARDING_PARAMETER_NODE: { // The `...` parameter in a method definition, like `def foo(...)`
            return translateSimpleKeyword<pm_forwarding_parameter_node, parser::ForwardArg>(node);
        }
        case PM_FORWARDING_SUPER_NODE: { // `super` with no `(...)`
            return translateSimpleKeyword<pm_forwarding_super_node, parser::ZSuper>(node);
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
            auto globalVarReadNode = reinterpret_cast<pm_global_variable_read_node *>(node);
            pm_location_t *loc = &globalVarReadNode->base.location;

            std::string_view name = parser.resolveConstant(globalVarReadNode->name);

            return make_unique<parser::GVar>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_GLOBAL_VARIABLE_WRITE_NODE: { // Regular assignment to a global variable, e.g. `$g = 1`
            return translateAssignment<pm_global_variable_write_node, parser::GVarLhs>(node);
        }
        case PM_HASH_NODE: { // A hash literal, like `{ a: 1, b: 2 }`
            auto usedForKeywordArgs = false;
            return translateHash(node, reinterpret_cast<pm_hash_node *>(node)->elements, usedForKeywordArgs);
        }
        case PM_HASH_PATTERN_NODE: { // An hash pattern such as the `{ k: Integer }` in the `h in { k: Integer }`
            auto hashPatternNode = reinterpret_cast<pm_hash_pattern_node *>(node);
            pm_location_t *loc = &hashPatternNode->base.location;

            auto prismElements = absl::MakeSpan(hashPatternNode->elements.nodes, hashPatternNode->elements.size);
            auto prismRestNode = hashPatternNode->rest;

            NodeVec sorbetElements{};
            sorbetElements.reserve(prismElements.size() + (prismRestNode != nullptr ? 1 : 0));

            translateMultiInto(sorbetElements, prismElements);
            if (prismRestNode != nullptr) {
                auto loc = parser.translateLocation(&prismRestNode->location);

                switch (PM_NODE_TYPE(prismRestNode)) {
                    case PM_ASSOC_SPLAT_NODE: {
                        sorbetElements.emplace_back(make_unique<parser::MatchRest>(loc, nullptr));
                        break;
                    }
                    case PM_NO_KEYWORDS_PARAMETER_NODE: {
                        sorbetElements.emplace_back(make_unique<parser::MatchNilPattern>(loc));
                        break;
                    }
                    default:
                        sorbetElements.emplace_back(translate(prismRestNode));
                }
            }

            return make_unique<parser::HashPattern>(parser.translateLocation(loc), std::move(sorbetElements));
        }
        case PM_IF_NODE: { // An `if` statement or modifier, like `if cond; ...; end` or `a.b if cond`
            auto ifNode = reinterpret_cast<pm_if_node *>(node);
            auto *loc = &ifNode->base.location;

            auto predicate = translate(ifNode->predicate);
            auto ifTrue = translate(reinterpret_cast<pm_node *>(ifNode->statements));
            auto ifFalse = translate(ifNode->consequent);

            return make_unique<parser::If>(parser.translateLocation(loc), std::move(predicate), std::move(ifTrue),
                                           std::move(ifFalse));
        }
        case PM_IN_NODE: { // An `in` pattern such as in a `case` statement, or as a standalone expression.
            auto inNode = reinterpret_cast<pm_in_node *>(node);
            auto *loc = &inNode->base.location;

            auto sorbetPattern = translate(inNode->pattern);

            auto inlineIfSingle = true;
            auto statements = translateStatements(inNode->statements, inlineIfSingle);

            return make_unique<parser::InPattern>(parser.translateLocation(loc), std::move(sorbetPattern), nullptr,
                                                  std::move(statements));
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
            auto instanceVarNode = reinterpret_cast<pm_instance_variable_read_node *>(node);
            pm_location_t *loc = &instanceVarNode->base.location;

            std::string_view name = parser.resolveConstant(instanceVarNode->name);

            return make_unique<parser::IVar>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_INSTANCE_VARIABLE_WRITE_NODE: { // Regular assignment to an instance variable, e.g. `@iv = 1`
            return translateAssignment<pm_instance_variable_write_node, parser::IVarLhs>(node);
        }
        case PM_INTEGER_NODE: { // An integer literal, e.g. `123`
            auto intNode = reinterpret_cast<pm_integer_node *>(node);
            pm_location_t *loc = &intNode->base.location;

            // Will only work for positive, 32-bit integers
            return make_unique<parser::Integer>(parser.translateLocation(loc), std::to_string(intNode->value.value));
        }
        case PM_INTERPOLATED_STRING_NODE: { // An interpolated string like `"foo #{bar} baz"`
            auto interpolatedStringNode = reinterpret_cast<pm_interpolated_string_node *>(node);
            pm_location_t *loc = &interpolatedStringNode->base.location;

            auto sorbetParts = translateMulti(interpolatedStringNode->parts);

            return make_unique<parser::DString>(parser.translateLocation(loc), std::move(sorbetParts));
        }
        case PM_IT_LOCAL_VARIABLE_READ_NODE: { // The `it` implicit parameter added in Ruby 3.4, e.g. `a.map { it + 1 }`
            [[fallthrough]];
        }
        case PM_IT_PARAMETERS_NODE: { // Used in a parameter list for lambdas that the `it` implicit parameter.
            // See Prism::ParserStorage::ParsedRubyVersion
            unreachable("The `it` keyword was introduced in Ruby 3.4, which isn't supported by Sorbet yet.");
        }
        case PM_KEYWORD_HASH_NODE: { // A hash of keyword arguments, like `foo(a: 1, b: 2)`
            auto usedForKeywordArgs = true;
            return translateHash(node, reinterpret_cast<pm_keyword_hash_node *>(node)->elements, usedForKeywordArgs);
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: { // A keyword rest parameter, like `def foo(**kwargs)`
            // This doesn't include `**nil`, which is a `PM_NO_KEYWORDS_PARAMETER_NODE`.

            auto keywordRestParamNode = reinterpret_cast<pm_keyword_rest_parameter_node *>(node);
            pm_location_t *loc = &keywordRestParamNode->base.location;

            core::NameRef sorbetName;
            if (auto prismName = keywordRestParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named keyword rest parameter, like `def foo(**kwargs)`
                auto name = parser.resolveConstant(prismName);
                sorbetName = gs.enterNameUTF8(name);
            } else { // An anonymous keyword rest parameter, like `def foo(**)`
                sorbetName = gs.freshNameUnique(core::UniqueNameKind::Parser, core::Names::starStar(), ++uniqueCounter);
            }

            return make_unique<parser::Kwrestarg>(parser.translateLocation(loc), sorbetName);
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
            auto localVarReadNode = reinterpret_cast<pm_local_variable_read_node *>(node);
            pm_location_t *loc = &localVarReadNode->base.location;

            std::string_view name = parser.resolveConstant(localVarReadNode->name);

            return make_unique<parser::LVar>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_LOCAL_VARIABLE_TARGET_NODE: { // Left-hand side of an multi-assignment
            auto localVarTargetNode = reinterpret_cast<pm_local_variable_target_node *>(node);
            pm_location_t *loc = &localVarTargetNode->base.location;

            std::string_view name = parser.resolveConstant(localVarTargetNode->name);

            return make_unique<parser::LVarLhs>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_LOCAL_VARIABLE_WRITE_NODE: { // Regular assignment to a local variable, e.g. `local = 1`
            return translateAssignment<pm_local_variable_write_node, parser::LVarLhs>(node);
        }
        case PM_MODULE_NODE: { // Modules declarations, like `module A::B::C; ...; end`
            auto moduleNode = reinterpret_cast<pm_module_node *>(node);
            pm_location_t *loc = &moduleNode->base.location;
            pm_location_t *declLoc = &moduleNode->module_keyword_loc;

            auto name = translate(moduleNode->constant_path);
            auto body = translate(moduleNode->body);

            return make_unique<parser::Module>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                               std::move(name), std::move(body));
        }
        case PM_MULTI_WRITE_NODE: { // Multi-assignment, like `a, b = 1, 2`
            auto multiWriteNode = reinterpret_cast<pm_multi_write_node *>(node);
            pm_location_t *loc = &multiWriteNode->base.location;

            // Left-hand side of the assignment
            auto prismLefts = absl::MakeSpan(multiWriteNode->lefts.nodes, multiWriteNode->lefts.size);
            auto prismRights = absl::MakeSpan(multiWriteNode->rights.nodes, multiWriteNode->rights.size);
            auto prismSplat = multiWriteNode->rest;

            NodeVec sorbetLhs{};
            sorbetLhs.reserve(prismLefts.size() + prismRights.size() + (prismSplat != nullptr ? 1 : 0));

            translateMultiInto(sorbetLhs, prismLefts);

            if (prismSplat != nullptr) {
                // This requires separate handling from the `PM_SPLAT_NODE` because it
                // has a different Sorbet node type, `parser::SplatLhs`
                auto splatNode = reinterpret_cast<pm_splat_node *>(prismSplat);
                pm_location_t *loc = &splatNode->base.location;

                auto expr = translate(splatNode->expression);

                sorbetLhs.emplace_back(make_unique<parser::SplatLhs>(parser.translateLocation(loc), std::move(expr)));
            }

            translateMultiInto(sorbetLhs, prismRights);

            auto mlhs = make_unique<parser::Mlhs>(parser.translateLocation(loc), std::move(sorbetLhs));

            // Right-hand side of the assignment
            auto value = translate(multiWriteNode->value);

            return make_unique<parser::Masgn>(parser.translateLocation(loc), std::move(mlhs), std::move(value));
        }
        case PM_NEXT_NODE: { // A `next` statement, e.g. `next`, `next 1, 2, 3`
            auto nextNode = reinterpret_cast<pm_next_node *>(node);
            pm_location_t *loc = &nextNode->base.location;

            auto arguments = translateArguments(nextNode->arguments);

            return make_unique<parser::Next>(parser.translateLocation(loc), std::move(arguments));
        }
        case PM_NIL_NODE: { // The `nil` keyword
            return translateSimpleKeyword<pm_nil_node, parser::Nil>(node);
        }
        case PM_NO_KEYWORDS_PARAMETER_NODE: { // `**nil`, such as in `def foo(**nil)` or `h in { k: v, **nil}`
            unreachable("PM_NO_KEYWORDS_PARAMETER_NODE is handled separately in `PM_HASH_PATTERN_NODE` and "
                        "`PM_PARAMETERS_NODE`.");
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: { // An optional keyword parameter, like `def foo(a: 1)`
            auto optionalKeywordParamNode = reinterpret_cast<pm_optional_keyword_parameter_node *>(node);
            pm_location_t *loc = &optionalKeywordParamNode->base.location;
            pm_location_t *nameLoc = &optionalKeywordParamNode->name_loc;

            std::string_view name = parser.resolveConstant(optionalKeywordParamNode->name);
            unique_ptr<parser::Node> value = translate(optionalKeywordParamNode->value);

            return make_unique<parser::Kwoptarg>(parser.translateLocation(loc), gs.enterNameUTF8(name),
                                                 parser.translateLocation(nameLoc), std::move(value));
        }
        case PM_OPTIONAL_PARAMETER_NODE: { // An optional positional parameter, like `def foo(a = 1)`
            auto optionalParamNode = reinterpret_cast<pm_optional_parameter_node *>(node);
            pm_location_t *loc = &optionalParamNode->base.location;
            pm_location_t *nameLoc = &optionalParamNode->name_loc;

            std::string_view name = parser.resolveConstant(optionalParamNode->name);
            auto value = translate(optionalParamNode->value);

            return make_unique<parser::Optarg>(parser.translateLocation(loc), gs.enterNameUTF8(name),
                                               parser.translateLocation(nameLoc), std::move(value));
        }
        case PM_OR_NODE: { // operator `||` and `or`
            auto orNode = reinterpret_cast<pm_or_node *>(node);
            auto *loc = &orNode->base.location;

            auto left = translate(orNode->left);
            auto right = translate(orNode->right);

            return make_unique<parser::Or>(parser.translateLocation(loc), std::move(left), std::move(right));
        }
        case PM_PARAMETERS_NODE: { // The parameters declared at the top of a PM_DEF_NODE
            auto paramsNode = reinterpret_cast<pm_parameters_node *>(node);
            pm_location_t *loc = &paramsNode->base.location;

            auto requireds = absl::MakeSpan(paramsNode->requireds.nodes, paramsNode->requireds.size);
            auto optionals = absl::MakeSpan(paramsNode->optionals.nodes, paramsNode->optionals.size);
            auto keywords = absl::MakeSpan(paramsNode->keywords.nodes, paramsNode->keywords.size);

            parser::NodeVec params;

            auto restSize = paramsNode->rest == nullptr ? 0 : 1;
            auto kwrestSize = paramsNode->keyword_rest == nullptr ? 0 : 1;
            auto blockSize = paramsNode->block == nullptr ? 0 : 1;

            params.reserve(requireds.size() + optionals.size() + restSize + keywords.size() + kwrestSize + blockSize);

            translateMultiInto(params, requireds);
            translateMultiInto(params, optionals);

            if (paramsNode->rest != nullptr)
                params.emplace_back(translate(paramsNode->rest));

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
                        auto noKeywordsParamNode = reinterpret_cast<pm_no_keywords_parameter_node *>(prismKwRestNode);
                        pm_location_t *loc = &noKeywordsParamNode->base.location;

                        params.emplace_back(make_unique<parser::Kwnilarg>(parser.translateLocation(loc)));
                        break;
                    }
                    default:
                        unreachable("Unexpected keyword_rest node type in Hash pattern.");
                }
            }

            if (paramsNode->block != nullptr)
                params.emplace_back(translate(reinterpret_cast<pm_node *>(paramsNode->block)));

            return make_unique<parser::Args>(parser.translateLocation(loc), std::move(params));
        }
        case PM_PARENTHESES_NODE: { // A parethesized expression, e.g. `(a)`
            auto parensNode = reinterpret_cast<pm_parentheses_node *>(node);

            if (auto stmtsNode = parensNode->body; stmtsNode != nullptr) {
                auto inlineIfSingle = false;
                return translateStatements(reinterpret_cast<pm_statements_node *>(stmtsNode), inlineIfSingle);
            } else {
                return make_unique<parser::Begin>(parser.translateLocation(&parensNode->base.location), NodeVec{});
            }
        }
        case PM_PROGRAM_NODE: { // The root node of the parse tree, representing the entire program
            pm_program_node *programNode = reinterpret_cast<pm_program_node *>(node);

            return translate(reinterpret_cast<pm_node *>(programNode->statements));
        }
        case PM_RANGE_NODE: { // A Range literal, e.g. `a..b`, `a..`, `..b`, `a...b`, `a...`, `...b`
            auto rangeNode = reinterpret_cast<pm_range_node *>(node);
            pm_location_t *loc = &rangeNode->base.location;

            auto flags = static_cast<pm_range_flags>(rangeNode->base.flags);
            auto left = translate(rangeNode->left);
            auto right = translate(rangeNode->right);

            if (flags & PM_RANGE_FLAGS_EXCLUDE_END) { // `...`
                return make_unique<parser::ERange>(parser.translateLocation(loc), std::move(left), std::move(right));
            } else { // `..`
                return make_unique<parser::IRange>(parser.translateLocation(loc), std::move(left), std::move(right));
            }
        }
        case PM_RATIONAL_NODE: { // A rational number literal, e.g. `1r`
            // Note: in `1/2r`, only the `2r` is part of the `PM_RATIONAL_NODE`.
            // The `1/` is just divison of an integer.
            auto *rationalNode = reinterpret_cast<pm_rational_node *>(node);
            pm_location_t *loc = &rationalNode->base.location;

            const uint8_t *start = loc->start;
            const uint8_t *end = loc->end;

            // `-1` drops the trailing `r` end of the value
            auto value = std::string_view(reinterpret_cast<const char *>(start), end - start - 1);

            return make_unique<parser::Rational>(parser.translateLocation(loc), value);
        }
        case PM_REDO_NODE: { // The `redo` keyword
            return translateSimpleKeyword<pm_redo_node, parser::Redo>(node);
        }
        case PM_REGULAR_EXPRESSION_NODE: { // A regular expression literal, e.g. `/foo/`
            auto regularExpressionNode = reinterpret_cast<pm_regular_expression_node *>(node);
            pm_location_t *loc = &regularExpressionNode->base.location;
            pm_location_t *closingLoc = &regularExpressionNode->closing_loc;

            // Sorbet represents the regex content as a vector of string nodes
            parser::NodeVec parts;

            auto source = parser.extractString(&regularExpressionNode->unescaped);
            if (!source.empty()) {
                auto sourceStringNode =
                    make_unique<parser::String>(parser.translateLocation(loc), gs.enterNameUTF8(source));
                parts.emplace_back(std::move(sourceStringNode));
            }

            std::string_view optString;

            auto optStart = closingLoc->start + 1; // one character after the closing `/`
            auto optEnd = closingLoc->end;
            auto optLength = optEnd - optStart;

            // Some regexps have options, e.g. `/foo/i`
            if (optLength > 0) {
                optString = std::string_view(reinterpret_cast<const char *>(optStart), optLength);
            }

            auto opt = make_unique<parser::Regopt>(parser.translateLocation(closingLoc), optString);

            return make_unique<parser::Regexp>(parser.translateLocation(loc), std::move(parts), std::move(opt));
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: { // A required keyword parameter, like `def foo(a:)`
            auto requiredKeywordParamNode = reinterpret_cast<pm_required_keyword_parameter_node *>(node);
            pm_location_t *loc = &requiredKeywordParamNode->base.location;

            std::string_view name = parser.resolveConstant(requiredKeywordParamNode->name);

            return make_unique<parser::Kwarg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_REQUIRED_PARAMETER_NODE: { // A required positional parameter, like `def foo(a)`
            auto requiredParamNode = reinterpret_cast<pm_required_parameter_node *>(node);
            pm_location_t *loc = &requiredParamNode->base.location;

            std::string_view name = parser.resolveConstant(requiredParamNode->name);

            return make_unique<parser::Arg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_REST_PARAMETER_NODE: { // A rest parameter, like `def foo(*rest)`
            auto restParamNode = reinterpret_cast<pm_rest_parameter_node *>(node);
            pm_location_t *loc = &restParamNode->base.location;
            pm_location_t *nameLoc = &restParamNode->name_loc;

            core::NameRef sorbetName;
            if (auto prismName = restParamNode->name; prismName != PM_CONSTANT_ID_UNSET) {
                // A named rest parameter, like `def foo(*rest)`
                auto name = parser.resolveConstant(prismName);
                sorbetName = gs.enterNameUTF8(name);
            } else { // An anonymous rest parameter, like `def foo(*)`
                sorbetName = gs.freshNameUnique(core::UniqueNameKind::Parser, core::Names::star(), ++uniqueCounter);
                nameLoc = loc;
            }

            return make_unique<parser::Restarg>(parser.translateLocation(loc), sorbetName,
                                                parser.translateLocation(nameLoc));
        }
        case PM_RETURN_NODE: { // A `return` statement, like `return 1, 2, 3`
            auto returnNode = reinterpret_cast<pm_return_node *>(node);
            pm_location_t *loc = &returnNode->base.location;

            auto returnValues = translateArguments(returnNode->arguments);

            return make_unique<parser::Return>(parser.translateLocation(loc), std::move(returnValues));
        }
        case PM_RETRY_NODE: { // The `retry` keyword
            return translateSimpleKeyword<pm_retry_node, parser::Retry>(node);
        }
        case PM_SELF_NODE: { // The `self` keyword
            auto selfNode = reinterpret_cast<pm_self_node *>(node);
            pm_location_t *loc = &selfNode->base.location;

            return make_unique<parser::Self>(parser.translateLocation(loc));
        }
        case PM_SINGLETON_CLASS_NODE: { // A singleton class, like `class << self ... end`
            auto classNode = reinterpret_cast<pm_singleton_class_node *>(node);
            pm_location_t *loc = &classNode->base.location;
            pm_location_t *declLoc = &classNode->class_keyword_loc;

            auto expr = translate(classNode->expression);
            unique_ptr<parser::Node> body;

            if (classNode->body != nullptr) {
                body = translate(classNode->body);
            }

            return make_unique<parser::SClass>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                               std::move(expr), std::move(body));
        }
        case PM_SOURCE_ENCODING_NODE: { // The `__ENCODING__` keyword
            return translateSimpleKeyword<pm_source_encoding_node, parser::EncodingLiteral>(node);
        }
        case PM_SOURCE_FILE_NODE: { // The `__FILE__` keyword
            return translateSimpleKeyword<pm_source_file_node, parser::FileLiteral>(node);
        }
        case PM_SOURCE_LINE_NODE: { // The `__LINE__` keyword
            return translateSimpleKeyword<pm_source_line_node, parser::LineLiteral>(node);
        }
        case PM_SPLAT_NODE: { // A splat, like `*a` in an array literal, method call, or array pattern
            auto splatNode = reinterpret_cast<pm_splat_node *>(node);
            pm_location_t *loc = &splatNode->base.location;

            auto expr = translate(splatNode->expression);
            if (expr == nullptr) { // An anonymous splat like `f(*)`
                return make_unique<parser::ForwardedRestArg>(parser.translateLocation(loc));
            } else { // Splatting an expression like `f(*a)`
                return make_unique<parser::Splat>(parser.translateLocation(loc), std::move(expr));
            }
        }
        case PM_STATEMENTS_NODE: { // A sequence of statements, such a in a `begin` block, `()`, etc.
            auto inlineIfSingle = true;
            return translateStatements(reinterpret_cast<pm_statements_node *>(node), inlineIfSingle);
        }
        case PM_STRING_NODE: { // A string literal, e.g. `"foo"`
            auto strNode = reinterpret_cast<pm_string_node *>(node);
            pm_location_t *loc = &strNode->base.location;

            auto unescaped = &strNode->unescaped;
            auto source = parser.extractString(unescaped);

            // TODO: handle different string encodings
            return make_unique<parser::String>(parser.translateLocation(loc), gs.enterNameUTF8(source));
        }
        case PM_SUPER_NODE: { // The `super` keyword, like `super`, `super(a, b)`
            auto superNode = reinterpret_cast<pm_super_node *>(node);
            pm_location_t *loc = &superNode->base.location;

            auto returnValues = translateArguments(superNode->arguments);

            return make_unique<parser::Super>(parser.translateLocation(loc), std::move(returnValues));
        }
        case PM_SYMBOL_NODE: { // A symbol literal, e.g. `:foo`
            auto symNode = reinterpret_cast<pm_string_node *>(node);
            pm_location_t *loc = &symNode->base.location;

            auto unescaped = &symNode->unescaped;

            auto source = parser.extractString(unescaped);

            // TODO: can these have different encodings?
            return make_unique<parser::Symbol>(parser.translateLocation(loc), gs.enterNameUTF8(source));
        }
        case PM_TRUE_NODE: { // The `true` keyword
            return translateSimpleKeyword<pm_true_node, parser::True>(node);
        }
        case PM_UNLESS_NODE: { // An `unless` branch, either in a statement or modifier form.
            auto unlessNode = reinterpret_cast<pm_if_node *>(node);
            auto *loc = &unlessNode->base.location;

            auto predicate = translate(unlessNode->predicate);
            // These are flipped relative to `PM_IF_NODE`
            auto ifFalse = translate(reinterpret_cast<pm_node *>(unlessNode->statements));
            auto ifTrue = translate(unlessNode->consequent);

            return make_unique<parser::If>(parser.translateLocation(loc), std::move(predicate), std::move(ifTrue),
                                           std::move(ifFalse));
        }
        case PM_UNTIL_NODE: { // A `until` loop, like `until stop_condition; ...; end`
            auto untilNode = reinterpret_cast<pm_until_node *>(node);
            auto *loc = &untilNode->base.location;

            auto predicate = translate(untilNode->predicate);
            auto body = translate(reinterpret_cast<pm_node *>(untilNode->statements));

            return make_unique<parser::Until>(parser.translateLocation(loc), std::move(predicate), std::move(body));
        }
        case PM_WHEN_NODE: { // A `when` clause, as part of a `case` statement
            auto whenNode = reinterpret_cast<pm_when_node *>(node);
            auto *loc = &whenNode->base.location;

            auto sorbetConditions = translateMulti(whenNode->conditions);

            auto inlineIfSingle = true;
            auto statements = translateStatements(whenNode->statements, inlineIfSingle);

            return make_unique<parser::When>(parser.translateLocation(loc), std::move(sorbetConditions),
                                             std::move(statements));
        }
        case PM_WHILE_NODE: { // A `while` loop, like `while condition; ...; end`
            auto whileNode = reinterpret_cast<pm_while_node *>(node);
            auto *loc = &whileNode->base.location;

            auto inlineIfSingle = true;
            auto predicate = translate(whileNode->predicate);

            auto statements = translateStatements(whileNode->statements, inlineIfSingle);

            return make_unique<parser::While>(parser.translateLocation(loc), std::move(predicate),
                                              std::move(statements));
        }
        case PM_YIELD_NODE: { // The `yield` keyword, like `yield`, `yield 1, 2, 3`
            auto yieldNode = reinterpret_cast<pm_yield_node *>(node);
            pm_location_t *loc = &yieldNode->base.location;

            auto yieldArgs = translateArguments(yieldNode->arguments);

            return make_unique<parser::Yield>(parser.translateLocation(loc), std::move(yieldArgs));
        }

        case PM_ALIAS_GLOBAL_VARIABLE_NODE:
        case PM_ALIAS_METHOD_NODE:
        case PM_ALTERNATION_PATTERN_NODE:
        case PM_BACK_REFERENCE_READ_NODE:
        case PM_BLOCK_LOCAL_VARIABLE_NODE:
        case PM_CALL_TARGET_NODE:
        case PM_CAPTURE_PATTERN_NODE:
        case PM_CLASS_VARIABLE_TARGET_NODE:
        case PM_DEFINED_NODE:
        case PM_EMBEDDED_VARIABLE_NODE:
        case PM_ENSURE_NODE:
        case PM_FLIP_FLOP_NODE:
        case PM_FOR_NODE:
        case PM_GLOBAL_VARIABLE_TARGET_NODE:
        case PM_IMAGINARY_NODE:
        case PM_IMPLICIT_NODE:
        case PM_IMPLICIT_REST_NODE:
        case PM_INDEX_TARGET_NODE:
        case PM_INSTANCE_VARIABLE_TARGET_NODE:
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE:
        case PM_INTERPOLATED_REGULAR_EXPRESSION_NODE:
        case PM_INTERPOLATED_SYMBOL_NODE:
        case PM_INTERPOLATED_X_STRING_NODE:
        case PM_LAMBDA_NODE:
        case PM_MATCH_LAST_LINE_NODE:
        case PM_MATCH_PREDICATE_NODE:
        case PM_MATCH_REQUIRED_NODE:
        case PM_MATCH_WRITE_NODE:
        case PM_MISSING_NODE:
        case PM_MULTI_TARGET_NODE:
        case PM_NUMBERED_PARAMETERS_NODE:
        case PM_NUMBERED_REFERENCE_READ_NODE:
        case PM_PINNED_EXPRESSION_NODE:
        case PM_PINNED_VARIABLE_NODE:
        case PM_POST_EXECUTION_NODE:
        case PM_PRE_EXECUTION_NODE:
        case PM_RESCUE_MODIFIER_NODE:
        case PM_RESCUE_NODE:
        case PM_SHAREABLE_CONSTANT_NODE:
        case PM_UNDEF_NODE:
        case PM_X_STRING_NODE:
        case PM_SCOPE_NODE:
            auto type_id = PM_NODE_TYPE(node);
            auto type_name = pm_node_type_to_str(type_id);

            fmt::memory_buffer buf;
            fmt::format_to(std::back_inserter(buf), "Unimplemented node type {} (#{}).", type_name, type_id);
            std::string s = fmt::to_string(buf);

            auto fakeLocation = core::LocOffsets{0, 1};

            return make_unique<parser::String>(fakeLocation, gs.enterNameUTF8(s));
    }
}

std::unique_ptr<parser::Node> Translator::translate(const Node &node) {
    return translate(node.get_raw_node_pointer());
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

// Translates the given Prism elements, and appends them to the given `NodeVec` of Sorbet nodes.
void Translator::translateMultiInto(NodeVec &outSorbetNodes, absl::Span<pm_node_t *> prismNodes) {
    for (auto &prismNode : prismNodes) {
        unique_ptr<parser::Node> sorbetNode = translate(prismNode);
        outSorbetNodes.emplace_back(std::move(sorbetNode));
    }
}

// The legacy Sorbet parser doesn't have a counterpart to PM_ARGUMENTS_NODE to wrap the array
// of argument nodes. It just uses a NodeVec directly, which is what this function produces.
NodeVec Translator::translateArguments(pm_arguments_node *argsNode, size_t extraCapacity) {
    NodeVec results;

    if (argsNode == nullptr) {
        results.reserve(extraCapacity);
        return results;
    }

    auto prismArgs = absl::MakeSpan(argsNode->arguments.nodes, argsNode->arguments.size);
    results.reserve(prismArgs.size() + extraCapacity);

    translateMultiInto(results, prismArgs);

    return results;
}

// Translates the given Prism elements into a `parser::Hash`.
// The elements are are usually key/value pairs, but can also be Hash splats (`**`).
//
// This method is used by:
//   * PM_HASH_NODE (Hash literals)
//   * PM_KEYWORD_HASH_NODE (keyword arguments to a method call)
//
// @param node The node the elements came from. Only used for source location information.
// @param elements The Prism key/value pairs to be translated
// @param isUsedForKeywordArguments True if this hash represents keyword arguments to a function,
//                                  false if it represents a Hash literal.
std::unique_ptr<parser::Hash> Translator::translateHash(pm_node_t *node, pm_node_list_t elements,
                                                        bool isUsedForKeywordArguments) {
    pm_location_t *loc = &node->location;

    auto prismElements = absl::MakeSpan(elements.nodes, elements.size);

    parser::NodeVec sorbetElements{};
    sorbetElements.reserve(prismElements.size());

    for (auto &pair : prismElements) {
        if (PM_NODE_TYPE_P(pair, PM_ASSOC_SPLAT_NODE)) {
            auto prismSplatNode = reinterpret_cast<pm_assoc_splat_node *>(pair);
            pm_location_t *loc = &prismSplatNode->base.location;
            auto value = translate(prismSplatNode->value);

            std::unique_ptr<parser::Node> sorbetSplatNode;
            if (value == nullptr) { // An anonymous splat like `f(**)`
                sorbetSplatNode = make_unique<parser::ForwardedKwrestArg>(parser.translateLocation(loc));
            } else { // Splatting an expression like `f(**h)`
                sorbetSplatNode = make_unique<parser::Kwsplat>(parser.translateLocation(loc), std::move(value));
            }

            sorbetElements.emplace_back(std::move(sorbetSplatNode));
        } else {
            ENFORCE(PM_NODE_TYPE_P(pair, PM_ASSOC_NODE))
            unique_ptr<parser::Node> sorbetKVPair = translate(pair);
            sorbetElements.emplace_back(std::move(sorbetKVPair));
        }
    }

    return make_unique<parser::Hash>(parser.translateLocation(loc), isUsedForKeywordArguments,
                                     std::move(sorbetElements));
}

// Prism models a call with an explicit block argument as a `pm_call_node` that contains a `pm_block_node`.
// Sorbet's legacy parser models this the other way around, as a parent `Block` with a child `Send`.
//
// This function translates between the two, creating a `Block` node for the given `pm_block_node *`,
// and wrapping it around the given `Send` node.
std::unique_ptr<parser::Node> Translator::translateCallWithBlock(pm_block_node *prismBlockNode,
                                                                 std::unique_ptr<parser::Node> sendNode) {
    auto blockParametersNode = translate(prismBlockNode->parameters);
    auto body = translate(prismBlockNode->body);

    // TODO: what's the correct location to use for the Block?
    // TODO: do we have to adjust the location for the Send node?
    return make_unique<parser::Block>(sendNode->loc, std::move(sendNode), std::move(blockParametersNode),
                                      std::move(body));
}

// Translates the given Prism Statements Node into a `parser::Begin` node or an inlined `parser::Node`.
// @param inlineIfSingle If enabled and there's 1 child node, we skip the `Begin` and just return the one `parser::Node`
std::unique_ptr<parser::Node> Translator::translateStatements(pm_statements_node *stmtsNode, bool inlineIfSingle) {
    if (stmtsNode == nullptr)
        return nullptr;

    // For a single statement, do not create a `Begin` node and just return the statement, if that's enabled.
    if (inlineIfSingle && stmtsNode->body.size == 1) {
        return translate(stmtsNode->body.nodes[0]);
    }

    // For multiple statements, convert each statement and add them to the body of a Begin node
    parser::NodeVec sorbetStmts = translateMulti(stmtsNode->body);

    return make_unique<parser::Begin>(parser.translateLocation(&stmtsNode->base.location), std::move(sorbetStmts));
}

// Handles any one of the Prism nodes that models any kind of assignment to a constant or constant path.
template <typename PrismLhsNode, typename SorbetLHSNode>
unique_ptr<SorbetLHSNode> Translator::translateConst(PrismLhsNode *node) {
    static_assert(std::is_same_v<SorbetLHSNode, parser::Const> || std::is_same_v<SorbetLHSNode, parser::ConstLhs>,
                  "Invalid LHS type. Must be one of `parser::Const` or `parser::ConstLhs`.");

    auto constexpr isConstantPath = std::is_same_v<PrismLhsNode, pm_constant_path_target_node> ||
                                    std::is_same_v<PrismLhsNode, pm_constant_path_write_node> ||
                                    std::is_same_v<PrismLhsNode, pm_constant_path_node>;

    std::unique_ptr<parser::Node> parent;
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
            pm_location_t *delimiterLoc = &node->delimiter_loc; // The location of the `::`
            parent = make_unique<parser::Cbase>(parser.translateLocation(delimiterLoc));
        }
    } else { // Handle plain constants like `A`, that aren't part of a constant path.
        static_assert(std::is_same_v<PrismLhsNode, pm_constant_and_write_node> ||
                      std::is_same_v<PrismLhsNode, pm_constant_or_write_node> ||
                      std::is_same_v<PrismLhsNode, pm_constant_operator_write_node> ||
                      std::is_same_v<PrismLhsNode, pm_constant_target_node> ||
                      std::is_same_v<PrismLhsNode, pm_constant_read_node> ||
                      std::is_same_v<PrismLhsNode, pm_constant_write_node>);
        parent = nullptr;
    }

    pm_location_t *loc = &node->base.location;
    std::string_view name = parser.resolveConstant(node->name);

    return make_unique<SorbetLHSNode>(parser.translateLocation(loc), std::move(parent), gs.enterNameConstant(name));
}

// Translate a node that only has basic location information, and nothing else. E.g. `true`, `nil`, `it`.
template <typename PrismNode, typename SorbetNode>
std::unique_ptr<SorbetNode> Translator::translateSimpleKeyword(pm_node_t *untypedNode) {
    auto node = reinterpret_cast<PrismNode *>(untypedNode);
    pm_location_t *loc = &node->base.location;

    return make_unique<SorbetNode>(parser.translateLocation(loc));
}

}; // namespace sorbet::parser::Prism
