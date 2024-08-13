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
        case PM_ARRAY_NODE: {
            auto arrayNode = reinterpret_cast<pm_array_node *>(node);
            pm_location_t *loc = &arrayNode->base.location;

            auto prismElements = absl::MakeSpan(arrayNode->elements.nodes, arrayNode->elements.size);

            parser::NodeVec sorbetElements{};
            sorbetElements.reserve(prismElements.size());

            for (auto &prismElement : prismElements) {
                unique_ptr<parser::Node> sorbetElement = translate(prismElement);
                sorbetElements.emplace_back(std::move(sorbetElement));
            }

            return make_unique<parser::Array>(parser.translateLocation(loc), std::move(sorbetElements));
        }
        case PM_ASSOC_NODE: {
            auto assocNode = reinterpret_cast<pm_assoc_node *>(node);
            pm_location_t *loc = &assocNode->base.location;

            auto key = translate(assocNode->key);
            auto value = translate(assocNode->value);

            return make_unique<parser::Pair>(parser.translateLocation(loc), std::move(key), std::move(value));
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

            std::string_view name = parser.resolveConstant(blockParamNode->name);

            return make_unique<parser::Blockarg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_BLOCK_PARAMETERS_NODE: { // The parameters declared at the top of a PM_BLOCK_NODE
            auto paramsNode = reinterpret_cast<pm_block_parameters_node *>(node);
            return translate(reinterpret_cast<pm_node *>(paramsNode->parameters));
        }
        case PM_CALL_NODE: {
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

            auto sendNode =
                make_unique<parser::Send>(parser.translateLocation(loc), std::move(receiver), gs.enterNameUTF8(name),
                                          parser.translateLocation(messageLoc), std::move(args));

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
        case PM_CLASS_NODE: { // Class declarations, not including singleton class declarations (`class <<`)
            auto classNode = reinterpret_cast<pm_class_node *>(node);
            pm_location_t *loc = &classNode->base.location;
            pm_location_t *declLoc = &classNode->class_keyword_loc;

            auto name = translate(reinterpret_cast<pm_node *>(classNode->constant_path));
            std::unique_ptr<parser::Node> superclass;

            if (classNode->superclass != nullptr) {
                superclass = translate(reinterpret_cast<pm_node *>(classNode->superclass));
            }

            return make_unique<parser::Class>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                              std::move(name), std::move(superclass), nullptr);
        }
        case PM_CONSTANT_PATH_NODE: {
            // Part of a constant path, like the `A` in `A::B`. `B` is a `PM_CONSTANT_READ_NODE`
            auto constantPathNode = reinterpret_cast<pm_constant_path_node *>(node);
            pm_location_t *loc = &constantPathNode->base.location;

            std::string_view name = parser.resolveConstant(constantPathNode->name);

            std::unique_ptr<parser::Node> parent;
            if (constantPathNode->parent) {
                // This constant reference is chained onto another constant reference.
                // E.g. if `node` is pointing to `B`, then then `A` is the `parent` in `A::B::C`.
                parent = translate(reinterpret_cast<pm_node *>(constantPathNode->parent));
            } else { // This is a fully qualified constant reference, like `::A`.
                pm_location_t *delimiterLoc = &constantPathNode->delimiter_loc; // The location of the `::`
                parent = make_unique<parser::Cbase>(parser.translateLocation(delimiterLoc));
            }

            return make_unique<parser::Const>(parser.translateLocation(loc), std::move(parent),
                                              gs.enterNameConstant(name));
        }
        case PM_CONSTANT_READ_NODE: { // A single, unnested, non-fully qualified constant like "Foo"
            auto constantReadNode = reinterpret_cast<pm_constant_read_node *>(node);
            pm_location_t *loc = &constantReadNode->base.location;
            std::string_view name = parser.resolveConstant(constantReadNode->name);

            return make_unique<parser::Const>(parser.translateLocation(loc), nullptr, gs.enterNameConstant(name));
        }
        case PM_DEF_NODE: {
            auto defNode = reinterpret_cast<pm_def_node *>(node);
            pm_location_t *loc = &defNode->base.location;
            pm_location_t *declLoc = &defNode->def_keyword_loc;

            std::string_view name = parser.resolveConstant(defNode->name);
            auto params = translate(reinterpret_cast<pm_node *>(defNode->parameters));
            auto body = translate(defNode->body);

            return make_unique<parser::DefMethod>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                                  gs.enterNameUTF8(name), std::move(params), std::move(body));
        }
        case PM_ELSE_NODE: {
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
        case PM_FALSE_NODE: {
            auto falseNode = reinterpret_cast<pm_false_node *>(node);
            pm_location_t *loc = &falseNode->base.location;

            return make_unique<parser::False>(parser.translateLocation(loc));
        }
        case PM_FLOAT_NODE: {
            auto floatNode = reinterpret_cast<pm_float_node *>(node);
            pm_location_t *loc = &floatNode->base.location;

            return make_unique<parser::Float>(parser.translateLocation(loc), std::to_string(floatNode->value));
        }
        case PM_FORWARDING_SUPER_NODE: { // `super` with no `(...)`
            auto forwardingSuperNode = reinterpret_cast<pm_forwarding_super_node *>(node);
            pm_location_t *loc = &forwardingSuperNode->base.location;

            return make_unique<parser::ZSuper>(parser.translateLocation(loc));
        }
        case PM_HASH_NODE: {
            auto usedForKeywordArgs = false;
            return translateHash(node, reinterpret_cast<pm_hash_node *>(node)->elements, usedForKeywordArgs);
        }
        case PM_IF_NODE: {
            auto ifNode = reinterpret_cast<pm_if_node *>(node);
            auto *loc = &ifNode->base.location;

            auto predicate = translate(ifNode->predicate);
            auto ifTrue = translate(reinterpret_cast<pm_node *>(ifNode->statements));
            auto ifFalse = translate(ifNode->consequent);

            return make_unique<parser::If>(parser.translateLocation(loc), std::move(predicate), std::move(ifTrue),
                                           std::move(ifFalse));
        }
        case PM_INSTANCE_VARIABLE_READ_NODE: {
            auto instanceVarNode = reinterpret_cast<pm_instance_variable_read_node *>(node);
            pm_location_t *loc = &instanceVarNode->base.location;

            std::string_view name = parser.resolveConstant(instanceVarNode->name);

            return make_unique<parser::IVar>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_INSTANCE_VARIABLE_WRITE_NODE: {
            auto instanceVarNode = reinterpret_cast<pm_instance_variable_write_node *>(node);
            pm_location_t *loc = &instanceVarNode->base.location;

            std::string_view ivarName = parser.resolveConstant(instanceVarNode->name);
            auto lhs = make_unique<parser::IVarLhs>(parser.translateLocation(&instanceVarNode->name_loc),
                                                    gs.enterNameUTF8(ivarName));
            auto rhs = translate(instanceVarNode->value);

            return make_unique<parser::Assign>(parser.translateLocation(loc), std::move(lhs), std::move(rhs));
        }
        case PM_INTEGER_NODE: {
            auto intNode = reinterpret_cast<pm_integer_node *>(node);
            pm_location_t *loc = &intNode->base.location;

            // Will only work for positive, 32-bit integers
            return make_unique<parser::Integer>(parser.translateLocation(loc), std::to_string(intNode->value.value));
        }
        case PM_INTERPOLATED_STRING_NODE: { // An interpolated string like `"foo #{bar} baz"`
            auto interpolatedStringNode = reinterpret_cast<pm_interpolated_string_node *>(node);
            pm_location_t *loc = &interpolatedStringNode->base.location;

            auto prismStringParts =
                absl::MakeSpan(interpolatedStringNode->parts.nodes, interpolatedStringNode->parts.size);

            NodeVec sorbetParts{};
            sorbetParts.reserve(prismStringParts.size());

            for (auto &prismPart : prismStringParts) {
                unique_ptr<parser::Node> sorbetPart = translate(prismPart);
                sorbetParts.emplace_back(std::move(sorbetPart));
            }

            return make_unique<parser::DString>(parser.translateLocation(loc), std::move(sorbetParts));
        }
        case PM_KEYWORD_HASH_NODE: {
            auto usedForKeywordArgs = true;
            return translateHash(node, reinterpret_cast<pm_keyword_hash_node *>(node)->elements, usedForKeywordArgs);
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: {
            auto keywordRestParamNode = reinterpret_cast<pm_keyword_rest_parameter_node *>(node);
            pm_location_t *loc = &keywordRestParamNode->base.location;

            std::string_view name = parser.resolveConstant(keywordRestParamNode->name);

            return make_unique<parser::Kwrestarg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_NIL_NODE: {
            auto nilNode = reinterpret_cast<pm_nil_node *>(node);
            pm_location_t *loc = &nilNode->base.location;

            return make_unique<parser::Nil>(parser.translateLocation(loc));
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: {
            auto optionalKeywordParamNode = reinterpret_cast<pm_optional_keyword_parameter_node *>(node);
            pm_location_t *loc = &optionalKeywordParamNode->base.location;
            pm_location_t *nameLoc = &optionalKeywordParamNode->name_loc;

            std::string_view name = parser.resolveConstant(optionalKeywordParamNode->name);
            unique_ptr<parser::Node> value = translate(optionalKeywordParamNode->value);

            return make_unique<parser::Kwoptarg>(parser.translateLocation(loc), gs.enterNameUTF8(name),
                                                 parser.translateLocation(nameLoc), std::move(value));
        }
        case PM_OPTIONAL_PARAMETER_NODE: {
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

            for (auto &param : requireds) {
                unique_ptr<parser::Node> sorbetParam = translate(param);
                params.emplace_back(std::move(sorbetParam));
            }

            for (auto &param : optionals) {
                unique_ptr<parser::Node> sorbetParam = translate(param);
                params.emplace_back(std::move(sorbetParam));
            }

            if (paramsNode->rest != nullptr) {
                unique_ptr<parser::Node> rest = translate(paramsNode->rest);
                params.emplace_back(std::move(rest));
            }

            for (auto &param : keywords) {
                unique_ptr<parser::Node> sorbetParam = translate(param);
                params.emplace_back(std::move(sorbetParam));
            }

            if (paramsNode->keyword_rest != nullptr) {
                unique_ptr<parser::Node> keywordRest = translate(paramsNode->keyword_rest);
                params.emplace_back(std::move(keywordRest));
            }

            if (paramsNode->block != nullptr) {
                unique_ptr<parser::Node> block = translate(reinterpret_cast<pm_node *>(paramsNode->block));
                params.emplace_back(std::move(block));
            }

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
        case PM_PROGRAM_NODE: {
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
        case PM_RATIONAL_NODE: {
            auto *rationalNode = reinterpret_cast<pm_rational_node *>(node);
            pm_location_t *loc = &rationalNode->base.location;

            const uint8_t *start = rationalNode->numeric->location.start;
            const uint8_t *end = rationalNode->numeric->location.end;

            std::string value = std::string(reinterpret_cast<const char *>(start), end - start);

            return make_unique<parser::Rational>(parser.translateLocation(loc), value);
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: {
            auto requiredKeywordParamNode = reinterpret_cast<pm_required_keyword_parameter_node *>(node);
            pm_location_t *loc = &requiredKeywordParamNode->base.location;

            std::string_view name = parser.resolveConstant(requiredKeywordParamNode->name);

            return make_unique<parser::Kwarg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_REQUIRED_PARAMETER_NODE: {
            auto requiredParamNode = reinterpret_cast<pm_required_parameter_node *>(node);
            pm_location_t *loc = &requiredParamNode->base.location;

            std::string_view name = parser.resolveConstant(requiredParamNode->name);

            return make_unique<parser::Arg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_REST_PARAMETER_NODE: {
            auto restParamNode = reinterpret_cast<pm_rest_parameter_node *>(node);
            pm_location_t *loc = &restParamNode->base.location;
            pm_location_t *nameLoc = &restParamNode->name_loc;

            std::string_view name = parser.resolveConstant(restParamNode->name);

            return make_unique<parser::Restarg>(parser.translateLocation(loc), gs.enterNameUTF8(name),
                                                parser.translateLocation(nameLoc));
        }
        case PM_RETURN_NODE: {
            auto returnNode = reinterpret_cast<pm_return_node *>(node);
            pm_location_t *loc = &returnNode->base.location;

            auto returnValues = translateArguments(returnNode->arguments);

            return make_unique<parser::Return>(parser.translateLocation(loc), std::move(returnValues));
        }
        case PM_SELF_NODE: {
            auto selfNode = reinterpret_cast<pm_singleton_class_node *>(node);
            pm_location_t *loc = &selfNode->base.location;

            return make_unique<parser::Self>(parser.translateLocation(loc));
        }
        case PM_SINGLETON_CLASS_NODE: {
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
        case PM_SPLAT_NODE: {
            auto splatNode = reinterpret_cast<pm_splat_node *>(node);
            pm_location_t *loc = &splatNode->base.location;

            auto expr = translate(splatNode->expression);

            return make_unique<parser::Splat>(parser.translateLocation(loc), std::move(expr));
        }
        case PM_STATEMENTS_NODE: {
            auto inlineIfSingle = true;
            return translateStatements(reinterpret_cast<pm_statements_node *>(node), inlineIfSingle);
        }
        case PM_STRING_NODE: {
            auto strNode = reinterpret_cast<pm_string_node *>(node);
            pm_location_t *loc = &strNode->base.location;

            auto unescaped = &strNode->unescaped;
            auto source =
                std::string(reinterpret_cast<const char *>(pm_string_source(unescaped)), pm_string_length(unescaped));

            // TODO: handle different string encodings
            return make_unique<parser::String>(parser.translateLocation(loc), gs.enterNameUTF8(source));
        }
        case PM_SUPER_NODE: {
            auto superNode = reinterpret_cast<pm_super_node *>(node);
            pm_location_t *loc = &superNode->base.location;

            auto returnValues = translateArguments(superNode->arguments);

            return make_unique<parser::Super>(parser.translateLocation(loc), std::move(returnValues));
        }
        case PM_SYMBOL_NODE: {
            auto symNode = reinterpret_cast<pm_string_node *>(node);
            pm_location_t *loc = &symNode->base.location;

            auto unescaped = &symNode->unescaped;

            auto source =
                std::string(reinterpret_cast<const char *>(pm_string_source(unescaped)), pm_string_length(unescaped));

            // TODO: can these have different encodings?
            return make_unique<parser::Symbol>(parser.translateLocation(loc), gs.enterNameUTF8(source));
        }
        case PM_TRUE_NODE: {
            auto trueNode = reinterpret_cast<pm_true_node *>(node);
            pm_location_t *loc = &trueNode->base.location;

            return make_unique<parser::True>(parser.translateLocation(loc));
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
        case PM_UNTIL_NODE: {
            auto untilNode = reinterpret_cast<pm_until_node *>(node);
            auto *loc = &untilNode->base.location;

            auto predicate = translate(untilNode->predicate);
            auto body = translate(reinterpret_cast<pm_node *>(untilNode->statements));

            return make_unique<parser::Until>(parser.translateLocation(loc), std::move(predicate), std::move(body));
        }
        case PM_YIELD_NODE: {
            auto yieldNode = reinterpret_cast<pm_yield_node *>(node);
            pm_location_t *loc = &yieldNode->base.location;

            auto yieldArgs = translateArguments(yieldNode->arguments);

            return make_unique<parser::Yield>(parser.translateLocation(loc), std::move(yieldArgs));
        }

        case PM_ALIAS_GLOBAL_VARIABLE_NODE:
        case PM_ALIAS_METHOD_NODE:
        case PM_ALTERNATION_PATTERN_NODE:
        case PM_ARRAY_PATTERN_NODE:
        case PM_ASSOC_SPLAT_NODE:
        case PM_BACK_REFERENCE_READ_NODE:
        case PM_BEGIN_NODE:
        case PM_BLOCK_LOCAL_VARIABLE_NODE:
        case PM_BREAK_NODE:
        case PM_CALL_AND_WRITE_NODE:
        case PM_CALL_OPERATOR_WRITE_NODE:
        case PM_CALL_OR_WRITE_NODE:
        case PM_CALL_TARGET_NODE:
        case PM_CAPTURE_PATTERN_NODE:
        case PM_CASE_MATCH_NODE:
        case PM_CASE_NODE:
        case PM_CLASS_VARIABLE_AND_WRITE_NODE:
        case PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_CLASS_VARIABLE_OR_WRITE_NODE:
        case PM_CLASS_VARIABLE_READ_NODE:
        case PM_CLASS_VARIABLE_TARGET_NODE:
        case PM_CLASS_VARIABLE_WRITE_NODE:
        case PM_CONSTANT_AND_WRITE_NODE:
        case PM_CONSTANT_OPERATOR_WRITE_NODE:
        case PM_CONSTANT_OR_WRITE_NODE:
        case PM_CONSTANT_PATH_AND_WRITE_NODE:
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE:
        case PM_CONSTANT_PATH_OR_WRITE_NODE:
        case PM_CONSTANT_PATH_TARGET_NODE:
        case PM_CONSTANT_PATH_WRITE_NODE:
        case PM_CONSTANT_TARGET_NODE:
        case PM_CONSTANT_WRITE_NODE:
        case PM_DEFINED_NODE:
        case PM_EMBEDDED_VARIABLE_NODE:
        case PM_ENSURE_NODE:
        case PM_FIND_PATTERN_NODE:
        case PM_FLIP_FLOP_NODE:
        case PM_FOR_NODE:
        case PM_FORWARDING_ARGUMENTS_NODE:
        case PM_FORWARDING_PARAMETER_NODE:
        case PM_GLOBAL_VARIABLE_AND_WRITE_NODE:
        case PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_GLOBAL_VARIABLE_OR_WRITE_NODE:
        case PM_GLOBAL_VARIABLE_READ_NODE:
        case PM_GLOBAL_VARIABLE_TARGET_NODE:
        case PM_GLOBAL_VARIABLE_WRITE_NODE:
        case PM_HASH_PATTERN_NODE:
        case PM_IMAGINARY_NODE:
        case PM_IMPLICIT_NODE:
        case PM_IMPLICIT_REST_NODE:
        case PM_IN_NODE:
        case PM_INDEX_AND_WRITE_NODE:
        case PM_INDEX_OPERATOR_WRITE_NODE:
        case PM_INDEX_OR_WRITE_NODE:
        case PM_INDEX_TARGET_NODE:
        case PM_INSTANCE_VARIABLE_AND_WRITE_NODE:
        case PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_INSTANCE_VARIABLE_OR_WRITE_NODE:
        case PM_INSTANCE_VARIABLE_TARGET_NODE:
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE:
        case PM_INTERPOLATED_REGULAR_EXPRESSION_NODE:
        case PM_INTERPOLATED_SYMBOL_NODE:
        case PM_INTERPOLATED_X_STRING_NODE:
        case PM_IT_PARAMETERS_NODE:
        case PM_LAMBDA_NODE:
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE:
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE:
        case PM_LOCAL_VARIABLE_READ_NODE:
        case PM_LOCAL_VARIABLE_TARGET_NODE:
        case PM_LOCAL_VARIABLE_WRITE_NODE:
        case PM_MATCH_LAST_LINE_NODE:
        case PM_MATCH_PREDICATE_NODE:
        case PM_MATCH_REQUIRED_NODE:
        case PM_MATCH_WRITE_NODE:
        case PM_MISSING_NODE:
        case PM_MODULE_NODE:
        case PM_MULTI_TARGET_NODE:
        case PM_MULTI_WRITE_NODE:
        case PM_NEXT_NODE:
        case PM_NO_KEYWORDS_PARAMETER_NODE:
        case PM_NUMBERED_PARAMETERS_NODE:
        case PM_NUMBERED_REFERENCE_READ_NODE:
        case PM_PINNED_EXPRESSION_NODE:
        case PM_PINNED_VARIABLE_NODE:
        case PM_POST_EXECUTION_NODE:
        case PM_PRE_EXECUTION_NODE:
        case PM_REDO_NODE:
        case PM_REGULAR_EXPRESSION_NODE:
        case PM_RESCUE_MODIFIER_NODE:
        case PM_RESCUE_NODE:
        case PM_RETRY_NODE:
        case PM_SHAREABLE_CONSTANT_NODE:
        case PM_SOURCE_ENCODING_NODE:
        case PM_SOURCE_FILE_NODE:
        case PM_SOURCE_LINE_NODE:
        case PM_UNDEF_NODE:
        case PM_WHEN_NODE:
        case PM_WHILE_NODE:
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

    for (auto &prismArg : prismArgs) {
        unique_ptr<parser::Node> sorbetArg = translate(prismArg);
        results.emplace_back(std::move(sorbetArg));
    }

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
        unique_ptr<parser::Node> sorbetKVPair = translate(pair);
        sorbetElements.emplace_back(std::move(sorbetKVPair));
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
                                                                 std::unique_ptr<parser::Send> sendNode) {
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
    auto prismStmts = absl::MakeSpan(stmtsNode->body.nodes, stmtsNode->body.size);

    // For a single statement, do not create a `Begin` node and just return the statement, if that's enabled.
    if (inlineIfSingle && prismStmts.size() == 1) {
        return translate(reinterpret_cast<pm_node_t *>(prismStmts.front()));
    }

    // For multiple statements, convert each statement and add them to the body of a Begin node
    parser::NodeVec sorbetStmts;
    sorbetStmts.reserve(prismStmts.size());

    for (auto &statement : prismStmts) {
        unique_ptr<parser::Node> sorbetStmt = translate(statement);
        sorbetStmts.emplace_back(std::move(sorbetStmt));
    }

    return make_unique<parser::Begin>(parser.translateLocation(&stmtsNode->base.location), std::move(sorbetStmts));
}

}; // namespace sorbet::parser::Prism
