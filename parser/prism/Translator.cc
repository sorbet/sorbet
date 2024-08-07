#include "Translator.h"

#include "core/LocOffsets.h"

template class std::unique_ptr<sorbet::parser::Node>;

using std::make_unique;
using std::unique_ptr;

namespace sorbet::parser::Prism {

namespace {

core::LocOffsets locOffset(pm_location_t *loc, pm_parser_t *parser) {
    uint32_t locStart = static_cast<uint32_t>(loc->start - parser->start);
    uint32_t locEnd = static_cast<uint32_t>(loc->end - parser->start);

    return core::LocOffsets{locStart, locEnd};
}

std::string_view prismConstantName(pm_constant_id_t name, pm_parser_t *parser) {
    pm_constant_pool_t *constantPool = &parser->constant_pool;
    pm_constant_t *constant = pm_constant_pool_id_to_constant(constantPool, name);

    return std::string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

} // namespace

std::unique_ptr<parser::Node> Translator::convertPrismToSorbet(pm_node_t *node, pm_parser_t *parser,
                                                               core::GlobalState &gs) {
    switch (PM_NODE_TYPE(node)) {
        case PM_BLOCK_PARAMETER_NODE: {
            auto blockParamNode = reinterpret_cast<pm_block_parameter_node *>(node);
            pm_location_t *loc = &blockParamNode->base.location;

            std::string_view name = prismConstantName(blockParamNode->name, parser);

            return make_unique<parser::Blockarg>(locOffset(loc, parser), gs.enterNameUTF8(name));
        }
        case PM_DEF_NODE: {
            auto defNode = reinterpret_cast<pm_def_node *>(node);
            pm_location_t *loc = &defNode->base.location;
            pm_location_t *declLoc = &defNode->def_keyword_loc;

            std::string_view name = prismConstantName(defNode->name, parser);

            unique_ptr<parser::Node> params;
            unique_ptr<parser::Node> body;

            if (defNode->body != nullptr) {
                body = convertPrismToSorbet(defNode->body, parser, gs);
            }

            if (defNode->parameters != nullptr) {
                params = convertPrismToSorbet(reinterpret_cast<pm_node *>(defNode->parameters), parser, gs);
            }

            return make_unique<parser::DefMethod>(locOffset(loc, parser), locOffset(declLoc, parser),
                                                  gs.enterNameUTF8(name), std::move(params), std::move(body));
        }
        case PM_ELSE_NODE: {
            auto elseNode = reinterpret_cast<pm_else_node *>(node);

            if (elseNode->statements == nullptr)
                return nullptr;

            return convertPrismToSorbet(reinterpret_cast<pm_node *>(elseNode->statements), parser, gs);
        }
        case PM_FALSE_NODE: {
            auto falseNode = reinterpret_cast<pm_false_node *>(node);
            pm_location_t *loc = &falseNode->base.location;

            return make_unique<parser::False>(locOffset(loc, parser));
        }
        case PM_FLOAT_NODE: {
            auto floatNode = reinterpret_cast<pm_float_node *>(node);
            pm_location_t *loc = &floatNode->base.location;

            return make_unique<parser::Float>(locOffset(loc, parser), std::to_string(floatNode->value));
        }
        case PM_IF_NODE: {
            auto ifNode = reinterpret_cast<pm_if_node *>(node);
            auto *loc = &ifNode->base.location;

            auto predicate = convertPrismToSorbet(ifNode->predicate, parser, gs);

            std::unique_ptr<parser::Node> ifTrue;
            std::unique_ptr<parser::Node> ifFalse;

            if (ifNode->statements != nullptr) {
                ifTrue = convertPrismToSorbet(reinterpret_cast<pm_node *>(ifNode->statements), parser, gs);
            }

            if (ifNode->consequent != nullptr) {
                ifFalse = convertPrismToSorbet(ifNode->consequent, parser, gs);
            }

            return make_unique<parser::If>(locOffset(loc, parser), std::move(predicate), std::move(ifTrue),
                                           std::move(ifFalse));
        }
        case PM_INTEGER_NODE: {
            auto intNode = reinterpret_cast<pm_integer_node *>(node);
            pm_location_t *loc = &intNode->base.location;

            // Will only work for positive, 32-bit integers
            return make_unique<parser::Integer>(locOffset(loc, parser), std::to_string(intNode->value.value));
        }
        case PM_KEYWORD_REST_PARAMETER_NODE: {
            auto keywordRestParamNode = reinterpret_cast<pm_keyword_rest_parameter_node *>(node);
            pm_location_t *loc = &keywordRestParamNode->base.location;

            std::string_view name = prismConstantName(keywordRestParamNode->name, parser);

            return make_unique<parser::Kwrestarg>(locOffset(loc, parser), gs.enterNameUTF8(name));
        }
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE: {
            auto optionalKeywordParamNode = reinterpret_cast<pm_optional_keyword_parameter_node *>(node);
            pm_location_t *loc = &optionalKeywordParamNode->base.location;
            pm_location_t *nameLoc = &optionalKeywordParamNode->name_loc;

            std::string_view name = prismConstantName(optionalKeywordParamNode->name, parser);
            unique_ptr<parser::Node> value = convertPrismToSorbet(optionalKeywordParamNode->value, parser, gs);

            return make_unique<parser::Kwoptarg>(locOffset(loc, parser), gs.enterNameUTF8(name),
                                                 locOffset(nameLoc, parser), std::move(value));
        }
        case PM_OPTIONAL_PARAMETER_NODE: {
            auto optionalParamNode = reinterpret_cast<pm_optional_parameter_node *>(node);
            pm_location_t *loc = &optionalParamNode->base.location;
            pm_location_t *nameLoc = &optionalParamNode->name_loc;

            std::string_view name = prismConstantName(optionalParamNode->name, parser);
            auto value = convertPrismToSorbet(optionalParamNode->value, parser, gs);

            return make_unique<parser::Optarg>(locOffset(loc, parser), gs.enterNameUTF8(name),
                                               locOffset(nameLoc, parser), std::move(value));
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
                unique_ptr<parser::Node> sorbetParam = convertPrismToSorbet(param, parser, gs);
                params.emplace_back(std::move(sorbetParam));
            }

            for (auto &param : optionals) {
                unique_ptr<parser::Node> sorbetParam = convertPrismToSorbet(param, parser, gs);
                params.emplace_back(std::move(sorbetParam));
            }

            if (paramsNode->rest != nullptr) {
                unique_ptr<parser::Node> rest = convertPrismToSorbet(paramsNode->rest, parser, gs);
                params.emplace_back(std::move(rest));
            }

            for (auto &param : keywords) {
                unique_ptr<parser::Node> sorbetParam = convertPrismToSorbet(param, parser, gs);
                params.emplace_back(std::move(sorbetParam));
            }

            if (paramsNode->keyword_rest != nullptr) {
                unique_ptr<parser::Node> keywordRest = convertPrismToSorbet(paramsNode->keyword_rest, parser, gs);
                params.emplace_back(std::move(keywordRest));
            }

            if (paramsNode->block != nullptr) {
                unique_ptr<parser::Node> block =
                    convertPrismToSorbet(reinterpret_cast<pm_node *>(paramsNode->block), parser, gs);
                params.emplace_back(std::move(block));
            }

            return make_unique<parser::Args>(locOffset(loc, parser), std::move(params));
        }
        case PM_PROGRAM_NODE: {
            pm_program_node *programNode = reinterpret_cast<pm_program_node *>(node);

            return convertPrismToSorbet(reinterpret_cast<pm_node *>(programNode->statements), parser, gs);
        }
        case PM_RATIONAL_NODE: {
            auto *rationalNode = reinterpret_cast<pm_rational_node *>(node);
            pm_location_t *loc = &rationalNode->base.location;

            const uint8_t *start = rationalNode->numeric->location.start;
            const uint8_t *end = rationalNode->numeric->location.end;

            std::string value = std::string(reinterpret_cast<const char *>(start), end - start);

            return make_unique<parser::Rational>(locOffset(loc, parser), value);
        }
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE: {
            auto requiredKeywordParamNode = reinterpret_cast<pm_required_keyword_parameter_node *>(node);
            pm_location_t *loc = &requiredKeywordParamNode->base.location;

            std::string_view name = prismConstantName(requiredKeywordParamNode->name, parser);

            return make_unique<parser::Kwarg>(locOffset(loc, parser), gs.enterNameUTF8(name));
        }
        case PM_REQUIRED_PARAMETER_NODE: {
            auto requiredParamNode = reinterpret_cast<pm_required_parameter_node *>(node);
            pm_location_t *loc = &requiredParamNode->base.location;

            std::string_view name = prismConstantName(requiredParamNode->name, parser);

            return make_unique<parser::Arg>(locOffset(loc, parser), gs.enterNameUTF8(name));
        }
        case PM_REST_PARAMETER_NODE: {
            auto restParamNode = reinterpret_cast<pm_rest_parameter_node *>(node);
            pm_location_t *loc = &restParamNode->base.location;
            pm_location_t *nameLoc = &restParamNode->name_loc;

            std::string_view name = prismConstantName(restParamNode->name, parser);

            return make_unique<parser::Restarg>(locOffset(loc, parser), gs.enterNameUTF8(name),
                                                locOffset(nameLoc, parser));
        }
        case PM_STATEMENTS_NODE: {
            pm_statements_node *stmts_node = reinterpret_cast<pm_statements_node *>(node);

            auto stmts = absl::MakeSpan(stmts_node->body.nodes, stmts_node->body.size);

            // For a single statement, do not create a Begin node and just return the statement
            if (stmts.size() == 1) {
                return convertPrismToSorbet((pm_node *)stmts.front(), parser, gs);
            }

            // For multiple statements, convert each statement and add them to the body of a Begin node
            parser::NodeVec sorbetStmts;
            sorbetStmts.reserve(stmts.size());

            for (auto &node : stmts) {
                unique_ptr<parser::Node> convertedStmt = convertPrismToSorbet(node, parser, gs);
                sorbetStmts.emplace_back(std::move(convertedStmt));
            }

            auto *loc = &stmts_node->base.location;

            return make_unique<parser::Begin>(locOffset(loc, parser), std::move(sorbetStmts));
        }
        case PM_STRING_NODE: {
            auto strNode = reinterpret_cast<pm_string_node *>(node);
            pm_location_t *loc = &strNode->base.location;

            auto unescaped = &strNode->unescaped;
            auto source =
                std::string(reinterpret_cast<const char *>(pm_string_source(unescaped)), pm_string_length(unescaped));

            // TODO: handle different string encodings
            return make_unique<parser::String>(locOffset(loc, parser), gs.enterNameUTF8(source));
        }
        case PM_TRUE_NODE: {
            auto trueNode = reinterpret_cast<pm_true_node *>(node);
            pm_location_t *loc = &trueNode->base.location;

            return make_unique<parser::True>(locOffset(loc, parser));
        }

        case PM_ALIAS_GLOBAL_VARIABLE_NODE:
        case PM_ALIAS_METHOD_NODE:
        case PM_ALTERNATION_PATTERN_NODE:
        case PM_AND_NODE:
        case PM_ARGUMENTS_NODE:
        case PM_ARRAY_NODE:
        case PM_ARRAY_PATTERN_NODE:
        case PM_ASSOC_NODE:
        case PM_ASSOC_SPLAT_NODE:
        case PM_BACK_REFERENCE_READ_NODE:
        case PM_BEGIN_NODE:
        case PM_BLOCK_ARGUMENT_NODE:
        case PM_BLOCK_LOCAL_VARIABLE_NODE:
        case PM_BLOCK_NODE:
        case PM_BLOCK_PARAMETERS_NODE:
        case PM_BREAK_NODE:
        case PM_CALL_AND_WRITE_NODE:
        case PM_CALL_NODE:
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
        case PM_CONSTANT_PATH_NODE:
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE:
        case PM_CONSTANT_PATH_OR_WRITE_NODE:
        case PM_CONSTANT_PATH_TARGET_NODE:
        case PM_CONSTANT_PATH_WRITE_NODE:
        case PM_CONSTANT_READ_NODE:
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
        case PM_HASH_NODE:
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
        case PM_SYMBOL_NODE:
        case PM_UNDEF_NODE:
        case PM_UNLESS_NODE:
        case PM_UNTIL_NODE:
        case PM_WHEN_NODE:
        case PM_WHILE_NODE:
        case PM_X_STRING_NODE:
        case PM_YIELD_NODE:
        case PM_SCOPE_NODE:
            std::unique_ptr<parser::Node> ast;
            return ast;
    }
}

}; // namespace sorbet::parser::Prism
