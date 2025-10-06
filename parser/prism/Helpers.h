#ifndef SORBET_PARSER_PRISM_HELPERS_H
#define SORBET_PARSER_PRISM_HELPERS_H

#include <string_view>
#include <type_traits>
#include <vector>
#include <cstdlib>
#include "core/LocOffsets.h"
extern "C" {
#include "prism.h"
}

template <> struct fmt::formatter<pm_node_type> : formatter<const char *> {
    auto format(pm_node_type type, format_context &ctx) const {
        return formatter<const char *>::format(pm_node_type_to_str(type), ctx);
    }
};

namespace sorbet::parser::Prism {

// This templated `PrismNodeTypeHelper` type and its specializations work as a lookup-table that associates
// Prism node types (C struct names) with their corresponding `pm_node_type_t` enum values.
template <typename Type> struct PrismNodeTypeHelper {};

#define DEF_TYPE_HELPER(pm_node_type, typeid)              \
    template <> struct PrismNodeTypeHelper<pm_node_type> { \
        static constexpr pm_node_type_t TypeID = typeid;   \
    }

// clang-format off
DEF_TYPE_HELPER(pm_alias_global_variable_node_t,           PM_ALIAS_GLOBAL_VARIABLE_NODE);
DEF_TYPE_HELPER(pm_alias_method_node_t,                    PM_ALIAS_METHOD_NODE);
DEF_TYPE_HELPER(pm_alternation_pattern_node_t,             PM_ALTERNATION_PATTERN_NODE);
DEF_TYPE_HELPER(pm_and_node_t,                             PM_AND_NODE);
DEF_TYPE_HELPER(pm_arguments_node_t,                       PM_ARGUMENTS_NODE);
DEF_TYPE_HELPER(pm_array_node_t,                           PM_ARRAY_NODE);
DEF_TYPE_HELPER(pm_array_pattern_node_t,                   PM_ARRAY_PATTERN_NODE);
DEF_TYPE_HELPER(pm_assoc_node_t,                           PM_ASSOC_NODE);
DEF_TYPE_HELPER(pm_assoc_splat_node_t,                     PM_ASSOC_SPLAT_NODE);
DEF_TYPE_HELPER(pm_back_reference_read_node_t,             PM_BACK_REFERENCE_READ_NODE);
DEF_TYPE_HELPER(pm_begin_node_t,                           PM_BEGIN_NODE);
DEF_TYPE_HELPER(pm_block_argument_node_t,                  PM_BLOCK_ARGUMENT_NODE);
DEF_TYPE_HELPER(pm_block_local_variable_node_t,            PM_BLOCK_LOCAL_VARIABLE_NODE);
DEF_TYPE_HELPER(pm_block_node_t,                           PM_BLOCK_NODE);
DEF_TYPE_HELPER(pm_block_parameter_node_t,                 PM_BLOCK_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_block_parameters_node_t,                PM_BLOCK_PARAMETERS_NODE);
DEF_TYPE_HELPER(pm_break_node_t,                           PM_BREAK_NODE);
DEF_TYPE_HELPER(pm_call_and_write_node_t,                  PM_CALL_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_call_node_t,                            PM_CALL_NODE);
DEF_TYPE_HELPER(pm_call_operator_write_node_t,             PM_CALL_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_call_or_write_node_t,                   PM_CALL_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_call_target_node_t,                     PM_CALL_TARGET_NODE);
DEF_TYPE_HELPER(pm_capture_pattern_node_t,                 PM_CAPTURE_PATTERN_NODE);
DEF_TYPE_HELPER(pm_case_match_node_t,                      PM_CASE_MATCH_NODE);
DEF_TYPE_HELPER(pm_case_node_t,                            PM_CASE_NODE);
DEF_TYPE_HELPER(pm_class_node_t,                           PM_CLASS_NODE);
DEF_TYPE_HELPER(pm_class_variable_and_write_node_t,        PM_CLASS_VARIABLE_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_class_variable_operator_write_node_t,   PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_class_variable_or_write_node_t,         PM_CLASS_VARIABLE_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_class_variable_read_node_t,             PM_CLASS_VARIABLE_READ_NODE);
DEF_TYPE_HELPER(pm_class_variable_target_node_t,           PM_CLASS_VARIABLE_TARGET_NODE);
DEF_TYPE_HELPER(pm_class_variable_write_node_t,            PM_CLASS_VARIABLE_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_and_write_node_t,              PM_CONSTANT_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_operator_write_node_t,         PM_CONSTANT_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_or_write_node_t,               PM_CONSTANT_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_path_and_write_node_t,         PM_CONSTANT_PATH_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_path_node_t,                   PM_CONSTANT_PATH_NODE);
DEF_TYPE_HELPER(pm_constant_path_operator_write_node_t,    PM_CONSTANT_PATH_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_path_or_write_node_t,          PM_CONSTANT_PATH_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_path_target_node_t,            PM_CONSTANT_PATH_TARGET_NODE);
DEF_TYPE_HELPER(pm_constant_path_write_node_t,             PM_CONSTANT_PATH_WRITE_NODE);
DEF_TYPE_HELPER(pm_constant_read_node_t,                   PM_CONSTANT_READ_NODE);
DEF_TYPE_HELPER(pm_constant_target_node_t,                 PM_CONSTANT_TARGET_NODE);
DEF_TYPE_HELPER(pm_constant_write_node_t,                  PM_CONSTANT_WRITE_NODE);
DEF_TYPE_HELPER(pm_def_node_t,                             PM_DEF_NODE);
DEF_TYPE_HELPER(pm_defined_node_t,                         PM_DEFINED_NODE);
DEF_TYPE_HELPER(pm_else_node_t,                            PM_ELSE_NODE);
DEF_TYPE_HELPER(pm_embedded_statements_node_t,             PM_EMBEDDED_STATEMENTS_NODE);
DEF_TYPE_HELPER(pm_embedded_variable_node_t,               PM_EMBEDDED_VARIABLE_NODE);
DEF_TYPE_HELPER(pm_ensure_node_t,                          PM_ENSURE_NODE);
DEF_TYPE_HELPER(pm_false_node_t,                           PM_FALSE_NODE);
DEF_TYPE_HELPER(pm_find_pattern_node_t,                    PM_FIND_PATTERN_NODE);
DEF_TYPE_HELPER(pm_flip_flop_node_t,                       PM_FLIP_FLOP_NODE);
DEF_TYPE_HELPER(pm_float_node_t,                           PM_FLOAT_NODE);
DEF_TYPE_HELPER(pm_for_node_t,                             PM_FOR_NODE);
DEF_TYPE_HELPER(pm_forwarding_arguments_node_t,            PM_FORWARDING_ARGUMENTS_NODE);
DEF_TYPE_HELPER(pm_forwarding_parameter_node_t,            PM_FORWARDING_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_forwarding_super_node_t,                PM_FORWARDING_SUPER_NODE);
DEF_TYPE_HELPER(pm_global_variable_and_write_node_t,       PM_GLOBAL_VARIABLE_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_global_variable_operator_write_node_t,  PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_global_variable_or_write_node_t,        PM_GLOBAL_VARIABLE_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_global_variable_read_node_t,            PM_GLOBAL_VARIABLE_READ_NODE);
DEF_TYPE_HELPER(pm_global_variable_target_node_t,          PM_GLOBAL_VARIABLE_TARGET_NODE);
DEF_TYPE_HELPER(pm_global_variable_write_node_t,           PM_GLOBAL_VARIABLE_WRITE_NODE);
DEF_TYPE_HELPER(pm_hash_node_t,                            PM_HASH_NODE);
DEF_TYPE_HELPER(pm_hash_pattern_node_t,                    PM_HASH_PATTERN_NODE);
DEF_TYPE_HELPER(pm_if_node_t,                              PM_IF_NODE);
DEF_TYPE_HELPER(pm_imaginary_node_t,                       PM_IMAGINARY_NODE);
DEF_TYPE_HELPER(pm_implicit_node_t,                        PM_IMPLICIT_NODE);
DEF_TYPE_HELPER(pm_implicit_rest_node_t,                   PM_IMPLICIT_REST_NODE);
DEF_TYPE_HELPER(pm_in_node_t,                              PM_IN_NODE);
DEF_TYPE_HELPER(pm_index_and_write_node_t,                 PM_INDEX_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_index_operator_write_node_t,            PM_INDEX_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_index_or_write_node_t,                  PM_INDEX_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_index_target_node_t,                    PM_INDEX_TARGET_NODE);
DEF_TYPE_HELPER(pm_instance_variable_and_write_node_t,     PM_INSTANCE_VARIABLE_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_instance_variable_operator_write_node_t,PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_instance_variable_or_write_node_t,      PM_INSTANCE_VARIABLE_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_instance_variable_read_node_t,          PM_INSTANCE_VARIABLE_READ_NODE);
DEF_TYPE_HELPER(pm_instance_variable_target_node_t,        PM_INSTANCE_VARIABLE_TARGET_NODE);
DEF_TYPE_HELPER(pm_instance_variable_write_node_t,         PM_INSTANCE_VARIABLE_WRITE_NODE);
DEF_TYPE_HELPER(pm_integer_node_t,                         PM_INTEGER_NODE);
DEF_TYPE_HELPER(pm_interpolated_match_last_line_node_t,    PM_INTERPOLATED_MATCH_LAST_LINE_NODE);
DEF_TYPE_HELPER(pm_interpolated_regular_expression_node_t, PM_INTERPOLATED_REGULAR_EXPRESSION_NODE);
DEF_TYPE_HELPER(pm_interpolated_string_node_t,             PM_INTERPOLATED_STRING_NODE);
DEF_TYPE_HELPER(pm_interpolated_symbol_node_t,             PM_INTERPOLATED_SYMBOL_NODE);
DEF_TYPE_HELPER(pm_interpolated_x_string_node_t,           PM_INTERPOLATED_X_STRING_NODE);
DEF_TYPE_HELPER(pm_it_local_variable_read_node_t,          PM_IT_LOCAL_VARIABLE_READ_NODE);
DEF_TYPE_HELPER(pm_it_parameters_node_t,                   PM_IT_PARAMETERS_NODE);
DEF_TYPE_HELPER(pm_keyword_hash_node_t,                    PM_KEYWORD_HASH_NODE);
DEF_TYPE_HELPER(pm_keyword_rest_parameter_node_t,          PM_KEYWORD_REST_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_lambda_node_t,                          PM_LAMBDA_NODE);
DEF_TYPE_HELPER(pm_local_variable_and_write_node_t,        PM_LOCAL_VARIABLE_AND_WRITE_NODE);
DEF_TYPE_HELPER(pm_local_variable_operator_write_node_t,   PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE);
DEF_TYPE_HELPER(pm_local_variable_or_write_node_t,         PM_LOCAL_VARIABLE_OR_WRITE_NODE);
DEF_TYPE_HELPER(pm_local_variable_read_node_t,             PM_LOCAL_VARIABLE_READ_NODE);
DEF_TYPE_HELPER(pm_local_variable_target_node_t,           PM_LOCAL_VARIABLE_TARGET_NODE);
DEF_TYPE_HELPER(pm_local_variable_write_node_t,            PM_LOCAL_VARIABLE_WRITE_NODE);
DEF_TYPE_HELPER(pm_match_last_line_node_t,                 PM_MATCH_LAST_LINE_NODE);
DEF_TYPE_HELPER(pm_match_predicate_node_t,                 PM_MATCH_PREDICATE_NODE);
DEF_TYPE_HELPER(pm_match_required_node_t,                  PM_MATCH_REQUIRED_NODE);
DEF_TYPE_HELPER(pm_match_write_node_t,                     PM_MATCH_WRITE_NODE);
DEF_TYPE_HELPER(pm_missing_node_t,                         PM_MISSING_NODE);
DEF_TYPE_HELPER(pm_module_node_t,                          PM_MODULE_NODE);
DEF_TYPE_HELPER(pm_multi_target_node_t,                    PM_MULTI_TARGET_NODE);
DEF_TYPE_HELPER(pm_multi_write_node_t,                     PM_MULTI_WRITE_NODE);
DEF_TYPE_HELPER(pm_next_node_t,                            PM_NEXT_NODE);
DEF_TYPE_HELPER(pm_nil_node_t,                             PM_NIL_NODE);
DEF_TYPE_HELPER(pm_no_keywords_parameter_node_t,           PM_NO_KEYWORDS_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_numbered_parameters_node_t,             PM_NUMBERED_PARAMETERS_NODE);
DEF_TYPE_HELPER(pm_numbered_reference_read_node_t,         PM_NUMBERED_REFERENCE_READ_NODE);
DEF_TYPE_HELPER(pm_optional_keyword_parameter_node_t,      PM_OPTIONAL_KEYWORD_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_optional_parameter_node_t,              PM_OPTIONAL_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_or_node_t,                              PM_OR_NODE);
DEF_TYPE_HELPER(pm_parameters_node_t,                      PM_PARAMETERS_NODE);
DEF_TYPE_HELPER(pm_parentheses_node_t,                     PM_PARENTHESES_NODE);
DEF_TYPE_HELPER(pm_pinned_expression_node_t,               PM_PINNED_EXPRESSION_NODE);
DEF_TYPE_HELPER(pm_pinned_variable_node_t,                 PM_PINNED_VARIABLE_NODE);
DEF_TYPE_HELPER(pm_post_execution_node_t,                  PM_POST_EXECUTION_NODE);
DEF_TYPE_HELPER(pm_pre_execution_node_t,                   PM_PRE_EXECUTION_NODE);
DEF_TYPE_HELPER(pm_program_node_t,                         PM_PROGRAM_NODE);
DEF_TYPE_HELPER(pm_range_node_t,                           PM_RANGE_NODE);
DEF_TYPE_HELPER(pm_rational_node_t,                        PM_RATIONAL_NODE);
DEF_TYPE_HELPER(pm_redo_node_t,                            PM_REDO_NODE);
DEF_TYPE_HELPER(pm_regular_expression_node_t,              PM_REGULAR_EXPRESSION_NODE);
DEF_TYPE_HELPER(pm_required_keyword_parameter_node_t,      PM_REQUIRED_KEYWORD_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_required_parameter_node_t,              PM_REQUIRED_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_rescue_modifier_node_t,                 PM_RESCUE_MODIFIER_NODE);
DEF_TYPE_HELPER(pm_rescue_node_t,                          PM_RESCUE_NODE);
DEF_TYPE_HELPER(pm_rest_parameter_node_t,                  PM_REST_PARAMETER_NODE);
DEF_TYPE_HELPER(pm_retry_node_t,                           PM_RETRY_NODE);
DEF_TYPE_HELPER(pm_return_node_t,                          PM_RETURN_NODE);
DEF_TYPE_HELPER(pm_self_node_t,                            PM_SELF_NODE);
DEF_TYPE_HELPER(pm_shareable_constant_node_t,              PM_SHAREABLE_CONSTANT_NODE);
DEF_TYPE_HELPER(pm_singleton_class_node_t,                 PM_SINGLETON_CLASS_NODE);
DEF_TYPE_HELPER(pm_source_encoding_node_t,                 PM_SOURCE_ENCODING_NODE);
DEF_TYPE_HELPER(pm_source_file_node_t,                     PM_SOURCE_FILE_NODE);
DEF_TYPE_HELPER(pm_source_line_node_t,                     PM_SOURCE_LINE_NODE);
DEF_TYPE_HELPER(pm_splat_node_t,                           PM_SPLAT_NODE);
DEF_TYPE_HELPER(pm_statements_node_t,                      PM_STATEMENTS_NODE);
DEF_TYPE_HELPER(pm_string_node_t,                          PM_STRING_NODE);
DEF_TYPE_HELPER(pm_super_node_t,                           PM_SUPER_NODE);
DEF_TYPE_HELPER(pm_symbol_node_t,                          PM_SYMBOL_NODE);
DEF_TYPE_HELPER(pm_true_node_t,                            PM_TRUE_NODE);
DEF_TYPE_HELPER(pm_undef_node_t,                           PM_UNDEF_NODE);
DEF_TYPE_HELPER(pm_unless_node_t,                          PM_UNLESS_NODE);
DEF_TYPE_HELPER(pm_until_node_t,                           PM_UNTIL_NODE);
DEF_TYPE_HELPER(pm_when_node_t,                            PM_WHEN_NODE);
DEF_TYPE_HELPER(pm_while_node_t,                           PM_WHILE_NODE);
DEF_TYPE_HELPER(pm_x_string_node_t,                        PM_X_STRING_NODE);
DEF_TYPE_HELPER(pm_yield_node_t,                           PM_YIELD_NODE);
#undef DEF_TYPE_HELPER
// clang-format on

// Returns true if the given `T` is a Prism node types. All Prism node types start with a `pm_node_t base` member.
template <typename T> constexpr bool isPrismNode = std::is_same_v<decltype(T::base), pm_node_t>;

// Take a pointer to a Prism node "subclass" (a thing with an embedded `pm_node_t base` as its first member),
// and up-casts it back to a general `pm_node_t` pointer.
template <typename PrismNode> pm_node_t *up_cast(PrismNode *node) {
    static_assert(!std::is_same_v<PrismNode, pm_node_t>,
                  "There's no need to call `up_cast` here, because this is already a `pm_node_t`.");
    static_assert(isPrismNode<PrismNode>, "The `up_cast` function should only be called on Prism node pointers.");
    return reinterpret_cast<pm_node_t *>(node);
}

// Take a pointer to a type-erased `pm_node_t` and down-cast it to a pointer of a specific Prism node "subclass".
// In debug builds, this helper checks the node's type before casting, to ensure it's casted correctly.
template <typename PrismNode> PrismNode *down_cast(pm_node_t *anyNode) {
    static_assert(std::is_same_v<decltype(PrismNode::base), pm_node_t>,
                  "The `down_cast` function should only be called on Prism node pointers.");
    ENFORCE(anyNode == nullptr || PM_NODE_TYPE_P(anyNode, PrismNodeTypeHelper<PrismNode>::TypeID),
            "Failed to cast a Prism AST Node. Expected {} (#{}), but got {} (#{}).",
            pm_node_type_to_str(PrismNodeTypeHelper<PrismNode>::TypeID), PrismNodeTypeHelper<PrismNode>::TypeID,
            pm_node_type_to_str(PM_NODE_TYPE(anyNode)), PM_NODE_TYPE(anyNode));
    return reinterpret_cast<PrismNode *>(anyNode);
}

inline std::string_view cast_prism_string(const uint8_t *source, size_t length) {
    // Prism conservatively uses `const uint8_t *` for its string types, to support platforms with non-8-bit chars.
    // Sorbet can be a bit more lax, and just assume that characters are 8 bits long.
    static_assert(std::is_same_v<const unsigned char *, const uint8_t *>, "Sorbet assumes that `char` is 8 bits long");
    return std::string_view(reinterpret_cast<const char *>(source), length);
}

// Forward declarations
class Parser;

// Node creation helpers
class PMK {
public:
    // Parser management
    static void setParser(const Parser *parser);

    // Memory allocation and node initialization
    template <typename T> static T *allocateNode() {
        T *node = (T *)calloc(1, sizeof(T));
        return node;
    }
    static pm_node_t initializeBaseNode(pm_node_type_t type);

    // Basic node creators
    static pm_node_t *ConstantReadNode(const char *name);
    static pm_node_t *ConstantPathNode(core::LocOffsets loc, pm_node_t *parent, const char *name);
    static pm_node_t *SingleArgumentNode(pm_node_t *arg);
    static pm_node_t *Self(core::LocOffsets loc = core::LocOffsets::none());

    // Symbol and hash node creators
    static pm_node_t *Symbol(core::LocOffsets nameLoc, const char *name);
    static pm_node_t *SymbolFromConstant(core::LocOffsets nameLoc, pm_constant_id_t nameId);
    static pm_node_t *AssocNode(core::LocOffsets loc, pm_node_t *key, pm_node_t *value);
    static pm_node_t *Hash(core::LocOffsets loc, const std::vector<pm_node_t *> &pairs);
    static pm_node_t *KeywordHash(core::LocOffsets loc, const std::vector<pm_node_t *> &pairs);

    // Low-level method call creation
    static pm_call_node_t *createSendNode(pm_node_t *receiver, pm_constant_id_t method_id,
                                          pm_node_t *arguments, pm_location_t message_loc,
                                          pm_location_t full_loc, pm_location_t tiny_loc,
                                          pm_node_t *block = nullptr);

    // High-level method call builders (similar to ast::MK)
    static pm_node_t *Send(core::LocOffsets loc, pm_node_t *receiver, const char *method,
                          const std::vector<pm_node_t *> &args, pm_node_t *block = nullptr);
    static pm_node_t *Send0(core::LocOffsets loc, pm_node_t *receiver, const char *method);
    static pm_node_t *Send1(core::LocOffsets loc, pm_node_t *receiver, const char *method,
                           pm_node_t *arg1);

    // Utility functions
    static pm_constant_id_t addConstantToPool(const char *name);
    static pm_location_t getZeroWidthLocation();
    static pm_location_t convertLocOffsets(core::LocOffsets loc);

    // Debug helpers
    static void debugPrintLocation(const char *label, pm_location_t loc);

    // High-level node creators
    static pm_node_t *SorbetPrivateStatic();
    static pm_node_t *TSigWithoutRuntime();

    // T constant and method helpers
    static pm_node_t *T(core::LocOffsets loc);
    static pm_node_t *TUntyped(core::LocOffsets loc);
    static pm_node_t *TNilable(core::LocOffsets loc, pm_node_t *type);
    static pm_node_t *TAny(core::LocOffsets loc, const std::vector<pm_node_t *> &args);
    static pm_node_t *TAll(core::LocOffsets loc, const std::vector<pm_node_t *> &args);
    static pm_node_t *TTypeParameter(core::LocOffsets loc, pm_node_t *name);
    static pm_node_t *TProc(core::LocOffsets loc, pm_node_t *args, pm_node_t *returnType);
    static pm_node_t *TProcVoid(core::LocOffsets loc, pm_node_t *args);
    static pm_node_t *T_Array(core::LocOffsets loc);
    static pm_node_t *T_Class(core::LocOffsets loc);
    static pm_node_t *T_Enumerable(core::LocOffsets loc);
    static pm_node_t *T_Enumerator(core::LocOffsets loc);
    static pm_node_t *T_Hash(core::LocOffsets loc);
    static pm_node_t *T_Set(core::LocOffsets loc);
    static pm_node_t *T_Range(core::LocOffsets loc);

    // Utility functions for type checking
    static bool isTUntyped(pm_node_t *node);
};

} // namespace sorbet::parser::Prism

#endif // SORBET_PARSER_PRISM_HELPERS_H
