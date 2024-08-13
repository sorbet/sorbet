#include "Translator.h"

template class std::unique_ptr<sorbet::parser::Node>;

using std::make_unique;
using std::unique_ptr;

namespace sorbet::parser::Prism {

std::unique_ptr<parser::Node> Translator::translate(pm_node_t *node) {
    switch (PM_NODE_TYPE(node)) {
        case PM_ASSOC_NODE: {
            auto assocNode = reinterpret_cast<pm_assoc_node *>(node);
            pm_location_t *loc = &assocNode->base.location;

            auto key = translate(assocNode->key);
            auto value = translate(assocNode->value);

            return make_unique<parser::Pair>(parser.translateLocation(loc), std::move(key), std::move(value));
        }
        case PM_BLOCK_PARAMETER_NODE: {
            auto blockParamNode = reinterpret_cast<pm_block_parameter_node *>(node);
            pm_location_t *loc = &blockParamNode->base.location;

            std::string_view name = parser.resolveConstant(blockParamNode->name);

            return make_unique<parser::Blockarg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
        }
        case PM_CALL_NODE: {
            auto callNode = reinterpret_cast<pm_call_node *>(node);
            pm_location_t *loc = &callNode->base.location;
            pm_location_t *messageLoc = &callNode->message_loc;

            auto name = parser.resolveConstant(callNode->name);
            std::unique_ptr<parser::Node> receiver;
            parser::NodeVec args;

            return make_unique<parser::Send>(parser.translateLocation(loc), std::move(receiver), gs.enterNameUTF8(name),
                                             parser.translateLocation(messageLoc), std::move(args));
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

            unique_ptr<parser::Node> params;
            unique_ptr<parser::Node> body;

            if (defNode->body != nullptr) {
                body = translate(defNode->body);
            }

            if (defNode->parameters != nullptr) {
                params = translate(reinterpret_cast<pm_node *>(defNode->parameters));
            }

            return make_unique<parser::DefMethod>(parser.translateLocation(loc), parser.translateLocation(declLoc),
                                                  gs.enterNameUTF8(name), std::move(params), std::move(body));
        }
        case PM_ELSE_NODE: {
            auto elseNode = reinterpret_cast<pm_else_node *>(node);

            if (elseNode->statements == nullptr)
                return nullptr;

            return translate(reinterpret_cast<pm_node *>(elseNode->statements));
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
        case PM_HASH_NODE: {
            auto hashNode = reinterpret_cast<pm_hash_node *>(node);
            pm_location_t *loc = &hashNode->base.location;

            parser::NodeVec sorbetHashKVPairs{};

            auto keywordPairs = absl::MakeSpan(hashNode->elements.nodes, hashNode->elements.size);

            for (auto &pair : keywordPairs) {
                unique_ptr<parser::Node> sorbetKVPair = translate(pair);
                sorbetHashKVPairs.emplace_back(std::move(sorbetKVPair));
            }

            return make_unique<parser::Hash>(parser.translateLocation(loc), /*kwargs*/ false,
                                             std::move(sorbetHashKVPairs));
        }
        case PM_IF_NODE: {
            auto ifNode = reinterpret_cast<pm_if_node *>(node);
            auto *loc = &ifNode->base.location;

            auto predicate = translate(ifNode->predicate);

            std::unique_ptr<parser::Node> ifTrue;
            std::unique_ptr<parser::Node> ifFalse;

            if (ifNode->statements != nullptr) {
                ifTrue = translate(reinterpret_cast<pm_node *>(ifNode->statements));
            }

            if (ifNode->consequent != nullptr) {
                ifFalse = translate(ifNode->consequent);
            }

            return make_unique<parser::If>(parser.translateLocation(loc), std::move(predicate), std::move(ifTrue),
                                           std::move(ifFalse));
        }
        case PM_INTEGER_NODE: {
            auto intNode = reinterpret_cast<pm_integer_node *>(node);
            pm_location_t *loc = &intNode->base.location;

            // Will only work for positive, 32-bit integers
            return make_unique<parser::Integer>(parser.translateLocation(loc), std::to_string(intNode->value.value));
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: {
            auto keywordRestParamNode = reinterpret_cast<pm_keyword_rest_parameter_node *>(node);
            pm_location_t *loc = &keywordRestParamNode->base.location;

            std::string_view name = parser.resolveConstant(keywordRestParamNode->name);

            return make_unique<parser::Kwrestarg>(parser.translateLocation(loc), gs.enterNameUTF8(name));
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
        case PM_PARAMETERS_NODE: {
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
        case PM_PROGRAM_NODE: {
            pm_program_node *programNode = reinterpret_cast<pm_program_node *>(node);

            return translate(reinterpret_cast<pm_node *>(programNode->statements));
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
        case PM_STATEMENTS_NODE: {
            pm_statements_node *stmts_node = reinterpret_cast<pm_statements_node *>(node);

            auto stmts = absl::MakeSpan(stmts_node->body.nodes, stmts_node->body.size);

            // For a single statement, do not create a Begin node and just return the statement
            if (stmts.size() == 1) {
                return translate((pm_node *)stmts.front());
            }

            // For multiple statements, convert each statement and add them to the body of a Begin node
            parser::NodeVec sorbetStmts;
            sorbetStmts.reserve(stmts.size());

            for (auto &node : stmts) {
                unique_ptr<parser::Node> convertedStmt = translate(node);
                sorbetStmts.emplace_back(std::move(convertedStmt));
            }

            auto *loc = &stmts_node->base.location;

            return make_unique<parser::Begin>(parser.translateLocation(loc), std::move(sorbetStmts));
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

        case PM_ALIAS_GLOBAL_VARIABLE_NODE:
        case PM_ALIAS_METHOD_NODE:
        case PM_ALTERNATION_PATTERN_NODE:
        case PM_AND_NODE:
        case PM_ARGUMENTS_NODE:
        case PM_ARRAY_NODE:
        case PM_ARRAY_PATTERN_NODE:
        case PM_ASSOC_SPLAT_NODE:
        case PM_BACK_REFERENCE_READ_NODE:
        case PM_BEGIN_NODE:
        case PM_BLOCK_ARGUMENT_NODE:
        case PM_BLOCK_LOCAL_VARIABLE_NODE:
        case PM_BLOCK_NODE:
        case PM_BLOCK_PARAMETERS_NODE:
        case PM_BREAK_NODE:
        case PM_CALL_AND_WRITE_NODE:
        case PM_CALL_OPERATOR_WRITE_NODE:
        case PM_CALL_OR_WRITE_NODE:
        case PM_CALL_TARGET_NODE:
        case PM_CAPTURE_PATTERN_NODE:
        case PM_CASE_MATCH_NODE:
        case PM_CASE_NODE:
        case PM_CLASS_NODE:
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
        case PM_EMBEDDED_STATEMENTS_NODE:
        case PM_EMBEDDED_VARIABLE_NODE:
        case PM_ENSURE_NODE:
        case PM_FIND_PATTERN_NODE:
        case PM_FLIP_FLOP_NODE:
        case PM_FOR_NODE:
        case PM_FORWARDING_ARGUMENTS_NODE:
        case PM_FORWARDING_PARAMETER_NODE:
        case PM_FORWARDING_SUPER_NODE:
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
        case PM_INSTANCE_VARIABLE_READ_NODE:
        case PM_INSTANCE_VARIABLE_TARGET_NODE:
        case PM_INSTANCE_VARIABLE_WRITE_NODE:
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE:
        case PM_INTERPOLATED_REGULAR_EXPRESSION_NODE:
        case PM_INTERPOLATED_STRING_NODE:
        case PM_INTERPOLATED_SYMBOL_NODE:
        case PM_INTERPOLATED_X_STRING_NODE:
        case PM_IT_PARAMETERS_NODE:
        case PM_KEYWORD_HASH_NODE:
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
        case PM_NIL_NODE:
        case PM_NO_KEYWORDS_PARAMETER_NODE:
        case PM_NUMBERED_PARAMETERS_NODE:
        case PM_NUMBERED_REFERENCE_READ_NODE:
        case PM_OR_NODE:
        case PM_PARENTHESES_NODE:
        case PM_PINNED_EXPRESSION_NODE:
        case PM_PINNED_VARIABLE_NODE:
        case PM_POST_EXECUTION_NODE:
        case PM_PRE_EXECUTION_NODE:
        case PM_RANGE_NODE:
        case PM_REDO_NODE:
        case PM_REGULAR_EXPRESSION_NODE:
        case PM_RESCUE_MODIFIER_NODE:
        case PM_RESCUE_NODE:
        case PM_RETRY_NODE:
        case PM_RETURN_NODE:
        case PM_SELF_NODE:
        case PM_SHAREABLE_CONSTANT_NODE:
        case PM_SINGLETON_CLASS_NODE:
        case PM_SOURCE_ENCODING_NODE:
        case PM_SOURCE_FILE_NODE:
        case PM_SOURCE_LINE_NODE:
        case PM_SPLAT_NODE:
        case PM_SUPER_NODE:
        case PM_UNDEF_NODE:
        case PM_UNLESS_NODE:
        case PM_UNTIL_NODE:
        case PM_WHEN_NODE:
        case PM_WHILE_NODE:
        case PM_X_STRING_NODE:
        case PM_YIELD_NODE:
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

}; // namespace sorbet::parser::Prism
