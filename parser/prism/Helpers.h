#ifndef SORBET_PARSER_PRISM_HELPERS_H
#define SORBET_PARSER_PRISM_HELPERS_H

#include <type_traits>
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
// clang-format off
template <typename Type> struct PrismNodeTypeHelper {};
template <> struct PrismNodeTypeHelper<pm_alias_global_variable_node_t           > { static constexpr pm_node_type_t TypeID = PM_ALIAS_GLOBAL_VARIABLE_NODE;             };
template <> struct PrismNodeTypeHelper<pm_alias_method_node_t                    > { static constexpr pm_node_type_t TypeID = PM_ALIAS_METHOD_NODE;                      };
template <> struct PrismNodeTypeHelper<pm_alternation_pattern_node_t             > { static constexpr pm_node_type_t TypeID = PM_ALTERNATION_PATTERN_NODE;               };
template <> struct PrismNodeTypeHelper<pm_and_node_t                             > { static constexpr pm_node_type_t TypeID = PM_AND_NODE;                               };
template <> struct PrismNodeTypeHelper<pm_arguments_node_t                       > { static constexpr pm_node_type_t TypeID = PM_ARGUMENTS_NODE;                         };
template <> struct PrismNodeTypeHelper<pm_array_node_t                           > { static constexpr pm_node_type_t TypeID = PM_ARRAY_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_array_pattern_node_t                   > { static constexpr pm_node_type_t TypeID = PM_ARRAY_PATTERN_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_assoc_node_t                           > { static constexpr pm_node_type_t TypeID = PM_ASSOC_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_assoc_splat_node_t                     > { static constexpr pm_node_type_t TypeID = PM_ASSOC_SPLAT_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_back_reference_read_node_t             > { static constexpr pm_node_type_t TypeID = PM_BACK_REFERENCE_READ_NODE;               };
template <> struct PrismNodeTypeHelper<pm_begin_node_t                           > { static constexpr pm_node_type_t TypeID = PM_BEGIN_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_block_argument_node_t                  > { static constexpr pm_node_type_t TypeID = PM_BLOCK_ARGUMENT_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_block_local_variable_node_t            > { static constexpr pm_node_type_t TypeID = PM_BLOCK_LOCAL_VARIABLE_NODE;              };
template <> struct PrismNodeTypeHelper<pm_block_node_t                           > { static constexpr pm_node_type_t TypeID = PM_BLOCK_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_block_parameter_node_t                 > { static constexpr pm_node_type_t TypeID = PM_BLOCK_PARAMETER_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_block_parameters_node_t                > { static constexpr pm_node_type_t TypeID = PM_BLOCK_PARAMETERS_NODE;                  };
template <> struct PrismNodeTypeHelper<pm_break_node_t                           > { static constexpr pm_node_type_t TypeID = PM_BREAK_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_call_and_write_node_t                  > { static constexpr pm_node_type_t TypeID = PM_CALL_AND_WRITE_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_call_node_t                            > { static constexpr pm_node_type_t TypeID = PM_CALL_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_call_operator_write_node_t             > { static constexpr pm_node_type_t TypeID = PM_CALL_OPERATOR_WRITE_NODE;               };
template <> struct PrismNodeTypeHelper<pm_call_or_write_node_t                   > { static constexpr pm_node_type_t TypeID = PM_CALL_OR_WRITE_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_call_target_node_t                     > { static constexpr pm_node_type_t TypeID = PM_CALL_TARGET_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_capture_pattern_node_t                 > { static constexpr pm_node_type_t TypeID = PM_CAPTURE_PATTERN_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_case_match_node_t                      > { static constexpr pm_node_type_t TypeID = PM_CASE_MATCH_NODE;                        };
template <> struct PrismNodeTypeHelper<pm_case_node_t                            > { static constexpr pm_node_type_t TypeID = PM_CASE_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_class_node_t                           > { static constexpr pm_node_type_t TypeID = PM_CLASS_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_class_variable_and_write_node_t        > { static constexpr pm_node_type_t TypeID = PM_CLASS_VARIABLE_AND_WRITE_NODE;          };
template <> struct PrismNodeTypeHelper<pm_class_variable_operator_write_node_t   > { static constexpr pm_node_type_t TypeID = PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE;     };
template <> struct PrismNodeTypeHelper<pm_class_variable_or_write_node_t         > { static constexpr pm_node_type_t TypeID = PM_CLASS_VARIABLE_OR_WRITE_NODE;           };
template <> struct PrismNodeTypeHelper<pm_class_variable_read_node_t             > { static constexpr pm_node_type_t TypeID = PM_CLASS_VARIABLE_READ_NODE;               };
template <> struct PrismNodeTypeHelper<pm_class_variable_target_node_t           > { static constexpr pm_node_type_t TypeID = PM_CLASS_VARIABLE_TARGET_NODE;             };
template <> struct PrismNodeTypeHelper<pm_class_variable_write_node_t            > { static constexpr pm_node_type_t TypeID = PM_CLASS_VARIABLE_WRITE_NODE;              };
template <> struct PrismNodeTypeHelper<pm_constant_and_write_node_t              > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_AND_WRITE_NODE;                };
template <> struct PrismNodeTypeHelper<pm_constant_operator_write_node_t         > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_OPERATOR_WRITE_NODE;           };
template <> struct PrismNodeTypeHelper<pm_constant_or_write_node_t               > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_OR_WRITE_NODE;                 };
template <> struct PrismNodeTypeHelper<pm_constant_path_and_write_node_t         > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_PATH_AND_WRITE_NODE;           };
template <> struct PrismNodeTypeHelper<pm_constant_path_node_t                   > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_PATH_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_constant_path_operator_write_node_t    > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_PATH_OPERATOR_WRITE_NODE;      };
template <> struct PrismNodeTypeHelper<pm_constant_path_or_write_node_t          > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_PATH_OR_WRITE_NODE;            };
template <> struct PrismNodeTypeHelper<pm_constant_path_target_node_t            > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_PATH_TARGET_NODE;              };
template <> struct PrismNodeTypeHelper<pm_constant_path_write_node_t             > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_PATH_WRITE_NODE;               };
template <> struct PrismNodeTypeHelper<pm_constant_read_node_t                   > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_READ_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_constant_target_node_t                 > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_TARGET_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_constant_write_node_t                  > { static constexpr pm_node_type_t TypeID = PM_CONSTANT_WRITE_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_def_node_t                             > { static constexpr pm_node_type_t TypeID = PM_DEF_NODE;                               };
template <> struct PrismNodeTypeHelper<pm_defined_node_t                         > { static constexpr pm_node_type_t TypeID = PM_DEFINED_NODE;                           };
template <> struct PrismNodeTypeHelper<pm_else_node_t                            > { static constexpr pm_node_type_t TypeID = PM_ELSE_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_embedded_statements_node_t             > { static constexpr pm_node_type_t TypeID = PM_EMBEDDED_STATEMENTS_NODE;               };
template <> struct PrismNodeTypeHelper<pm_embedded_variable_node_t               > { static constexpr pm_node_type_t TypeID = PM_EMBEDDED_VARIABLE_NODE;                 };
template <> struct PrismNodeTypeHelper<pm_ensure_node_t                          > { static constexpr pm_node_type_t TypeID = PM_ENSURE_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_false_node_t                           > { static constexpr pm_node_type_t TypeID = PM_FALSE_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_find_pattern_node_t                    > { static constexpr pm_node_type_t TypeID = PM_FIND_PATTERN_NODE;                      };
template <> struct PrismNodeTypeHelper<pm_flip_flop_node_t                       > { static constexpr pm_node_type_t TypeID = PM_FLIP_FLOP_NODE;                         };
template <> struct PrismNodeTypeHelper<pm_float_node_t                           > { static constexpr pm_node_type_t TypeID = PM_FLOAT_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_for_node_t                             > { static constexpr pm_node_type_t TypeID = PM_FOR_NODE;                               };
template <> struct PrismNodeTypeHelper<pm_forwarding_arguments_node_t            > { static constexpr pm_node_type_t TypeID = PM_FORWARDING_ARGUMENTS_NODE;              };
template <> struct PrismNodeTypeHelper<pm_forwarding_parameter_node_t            > { static constexpr pm_node_type_t TypeID = PM_FORWARDING_PARAMETER_NODE;              };
template <> struct PrismNodeTypeHelper<pm_forwarding_super_node_t                > { static constexpr pm_node_type_t TypeID = PM_FORWARDING_SUPER_NODE;                  };
template <> struct PrismNodeTypeHelper<pm_global_variable_and_write_node_t       > { static constexpr pm_node_type_t TypeID = PM_GLOBAL_VARIABLE_AND_WRITE_NODE;         };
template <> struct PrismNodeTypeHelper<pm_global_variable_operator_write_node_t  > { static constexpr pm_node_type_t TypeID = PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE;    };
template <> struct PrismNodeTypeHelper<pm_global_variable_or_write_node_t        > { static constexpr pm_node_type_t TypeID = PM_GLOBAL_VARIABLE_OR_WRITE_NODE;          };
template <> struct PrismNodeTypeHelper<pm_global_variable_read_node_t            > { static constexpr pm_node_type_t TypeID = PM_GLOBAL_VARIABLE_READ_NODE;              };
template <> struct PrismNodeTypeHelper<pm_global_variable_target_node_t          > { static constexpr pm_node_type_t TypeID = PM_GLOBAL_VARIABLE_TARGET_NODE;            };
template <> struct PrismNodeTypeHelper<pm_global_variable_write_node_t           > { static constexpr pm_node_type_t TypeID = PM_GLOBAL_VARIABLE_WRITE_NODE;             };
template <> struct PrismNodeTypeHelper<pm_hash_node_t                            > { static constexpr pm_node_type_t TypeID = PM_HASH_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_hash_pattern_node_t                    > { static constexpr pm_node_type_t TypeID = PM_HASH_PATTERN_NODE;                      };
template <> struct PrismNodeTypeHelper<pm_if_node_t                              > { static constexpr pm_node_type_t TypeID = PM_IF_NODE;                                };
template <> struct PrismNodeTypeHelper<pm_imaginary_node_t                       > { static constexpr pm_node_type_t TypeID = PM_IMAGINARY_NODE;                         };
template <> struct PrismNodeTypeHelper<pm_implicit_node_t                        > { static constexpr pm_node_type_t TypeID = PM_IMPLICIT_NODE;                          };
template <> struct PrismNodeTypeHelper<pm_implicit_rest_node_t                   > { static constexpr pm_node_type_t TypeID = PM_IMPLICIT_REST_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_in_node_t                              > { static constexpr pm_node_type_t TypeID = PM_IN_NODE;                                };
template <> struct PrismNodeTypeHelper<pm_index_and_write_node_t                 > { static constexpr pm_node_type_t TypeID = PM_INDEX_AND_WRITE_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_index_operator_write_node_t            > { static constexpr pm_node_type_t TypeID = PM_INDEX_OPERATOR_WRITE_NODE;              };
template <> struct PrismNodeTypeHelper<pm_index_or_write_node_t                  > { static constexpr pm_node_type_t TypeID = PM_INDEX_OR_WRITE_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_index_target_node_t                    > { static constexpr pm_node_type_t TypeID = PM_INDEX_TARGET_NODE;                      };
template <> struct PrismNodeTypeHelper<pm_instance_variable_and_write_node_t     > { static constexpr pm_node_type_t TypeID = PM_INSTANCE_VARIABLE_AND_WRITE_NODE;       };
template <> struct PrismNodeTypeHelper<pm_instance_variable_operator_write_node_t> { static constexpr pm_node_type_t TypeID = PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE;  };
template <> struct PrismNodeTypeHelper<pm_instance_variable_or_write_node_t      > { static constexpr pm_node_type_t TypeID = PM_INSTANCE_VARIABLE_OR_WRITE_NODE;        };
template <> struct PrismNodeTypeHelper<pm_instance_variable_read_node_t          > { static constexpr pm_node_type_t TypeID = PM_INSTANCE_VARIABLE_READ_NODE;            };
template <> struct PrismNodeTypeHelper<pm_instance_variable_target_node_t        > { static constexpr pm_node_type_t TypeID = PM_INSTANCE_VARIABLE_TARGET_NODE;          };
template <> struct PrismNodeTypeHelper<pm_instance_variable_write_node_t         > { static constexpr pm_node_type_t TypeID = PM_INSTANCE_VARIABLE_WRITE_NODE;           };
template <> struct PrismNodeTypeHelper<pm_integer_node_t                         > { static constexpr pm_node_type_t TypeID = PM_INTEGER_NODE;                           };
template <> struct PrismNodeTypeHelper<pm_interpolated_match_last_line_node_t    > { static constexpr pm_node_type_t TypeID = PM_INTERPOLATED_MATCH_LAST_LINE_NODE;      };
template <> struct PrismNodeTypeHelper<pm_interpolated_regular_expression_node_t > { static constexpr pm_node_type_t TypeID = PM_INTERPOLATED_REGULAR_EXPRESSION_NODE;   };
template <> struct PrismNodeTypeHelper<pm_interpolated_string_node_t             > { static constexpr pm_node_type_t TypeID = PM_INTERPOLATED_STRING_NODE;               };
template <> struct PrismNodeTypeHelper<pm_interpolated_symbol_node_t             > { static constexpr pm_node_type_t TypeID = PM_INTERPOLATED_SYMBOL_NODE;               };
template <> struct PrismNodeTypeHelper<pm_interpolated_x_string_node_t           > { static constexpr pm_node_type_t TypeID = PM_INTERPOLATED_X_STRING_NODE;             };
template <> struct PrismNodeTypeHelper<pm_it_local_variable_read_node_t          > { static constexpr pm_node_type_t TypeID = PM_IT_LOCAL_VARIABLE_READ_NODE;            };
template <> struct PrismNodeTypeHelper<pm_it_parameters_node_t                   > { static constexpr pm_node_type_t TypeID = PM_IT_PARAMETERS_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_keyword_hash_node_t                    > { static constexpr pm_node_type_t TypeID = PM_KEYWORD_HASH_NODE;                      };
template <> struct PrismNodeTypeHelper<pm_keyword_rest_parameter_node_t          > { static constexpr pm_node_type_t TypeID = PM_KEYWORD_REST_PARAMETER_NODE;            };
template <> struct PrismNodeTypeHelper<pm_lambda_node_t                          > { static constexpr pm_node_type_t TypeID = PM_LAMBDA_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_local_variable_and_write_node_t        > { static constexpr pm_node_type_t TypeID = PM_LOCAL_VARIABLE_AND_WRITE_NODE;          };
template <> struct PrismNodeTypeHelper<pm_local_variable_operator_write_node_t   > { static constexpr pm_node_type_t TypeID = PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE;     };
template <> struct PrismNodeTypeHelper<pm_local_variable_or_write_node_t         > { static constexpr pm_node_type_t TypeID = PM_LOCAL_VARIABLE_OR_WRITE_NODE;           };
template <> struct PrismNodeTypeHelper<pm_local_variable_read_node_t             > { static constexpr pm_node_type_t TypeID = PM_LOCAL_VARIABLE_READ_NODE;               };
template <> struct PrismNodeTypeHelper<pm_local_variable_target_node_t           > { static constexpr pm_node_type_t TypeID = PM_LOCAL_VARIABLE_TARGET_NODE;             };
template <> struct PrismNodeTypeHelper<pm_local_variable_write_node_t            > { static constexpr pm_node_type_t TypeID = PM_LOCAL_VARIABLE_WRITE_NODE;              };
template <> struct PrismNodeTypeHelper<pm_match_last_line_node_t                 > { static constexpr pm_node_type_t TypeID = PM_MATCH_LAST_LINE_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_match_predicate_node_t                 > { static constexpr pm_node_type_t TypeID = PM_MATCH_PREDICATE_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_match_required_node_t                  > { static constexpr pm_node_type_t TypeID = PM_MATCH_REQUIRED_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_match_write_node_t                     > { static constexpr pm_node_type_t TypeID = PM_MATCH_WRITE_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_missing_node_t                         > { static constexpr pm_node_type_t TypeID = PM_MISSING_NODE;                           };
template <> struct PrismNodeTypeHelper<pm_module_node_t                          > { static constexpr pm_node_type_t TypeID = PM_MODULE_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_multi_target_node_t                    > { static constexpr pm_node_type_t TypeID = PM_MULTI_TARGET_NODE;                      };
template <> struct PrismNodeTypeHelper<pm_multi_write_node_t                     > { static constexpr pm_node_type_t TypeID = PM_MULTI_WRITE_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_next_node_t                            > { static constexpr pm_node_type_t TypeID = PM_NEXT_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_nil_node_t                             > { static constexpr pm_node_type_t TypeID = PM_NIL_NODE;                               };
template <> struct PrismNodeTypeHelper<pm_no_keywords_parameter_node_t           > { static constexpr pm_node_type_t TypeID = PM_NO_KEYWORDS_PARAMETER_NODE;             };
template <> struct PrismNodeTypeHelper<pm_numbered_parameters_node_t             > { static constexpr pm_node_type_t TypeID = PM_NUMBERED_PARAMETERS_NODE;               };
template <> struct PrismNodeTypeHelper<pm_numbered_reference_read_node_t         > { static constexpr pm_node_type_t TypeID = PM_NUMBERED_REFERENCE_READ_NODE;           };
template <> struct PrismNodeTypeHelper<pm_optional_keyword_parameter_node_t      > { static constexpr pm_node_type_t TypeID = PM_OPTIONAL_KEYWORD_PARAMETER_NODE;        };
template <> struct PrismNodeTypeHelper<pm_optional_parameter_node_t              > { static constexpr pm_node_type_t TypeID = PM_OPTIONAL_PARAMETER_NODE;                };
template <> struct PrismNodeTypeHelper<pm_or_node_t                              > { static constexpr pm_node_type_t TypeID = PM_OR_NODE;                                };
template <> struct PrismNodeTypeHelper<pm_parameters_node_t                      > { static constexpr pm_node_type_t TypeID = PM_PARAMETERS_NODE;                        };
template <> struct PrismNodeTypeHelper<pm_parentheses_node_t                     > { static constexpr pm_node_type_t TypeID = PM_PARENTHESES_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_pinned_expression_node_t               > { static constexpr pm_node_type_t TypeID = PM_PINNED_EXPRESSION_NODE;                 };
template <> struct PrismNodeTypeHelper<pm_pinned_variable_node_t                 > { static constexpr pm_node_type_t TypeID = PM_PINNED_VARIABLE_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_post_execution_node_t                  > { static constexpr pm_node_type_t TypeID = PM_POST_EXECUTION_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_pre_execution_node_t                   > { static constexpr pm_node_type_t TypeID = PM_PRE_EXECUTION_NODE;                     };
template <> struct PrismNodeTypeHelper<pm_program_node_t                         > { static constexpr pm_node_type_t TypeID = PM_PROGRAM_NODE;                           };
template <> struct PrismNodeTypeHelper<pm_range_node_t                           > { static constexpr pm_node_type_t TypeID = PM_RANGE_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_rational_node_t                        > { static constexpr pm_node_type_t TypeID = PM_RATIONAL_NODE;                          };
template <> struct PrismNodeTypeHelper<pm_redo_node_t                            > { static constexpr pm_node_type_t TypeID = PM_REDO_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_regular_expression_node_t              > { static constexpr pm_node_type_t TypeID = PM_REGULAR_EXPRESSION_NODE;                };
template <> struct PrismNodeTypeHelper<pm_required_keyword_parameter_node_t      > { static constexpr pm_node_type_t TypeID = PM_REQUIRED_KEYWORD_PARAMETER_NODE;        };
template <> struct PrismNodeTypeHelper<pm_required_parameter_node_t              > { static constexpr pm_node_type_t TypeID = PM_REQUIRED_PARAMETER_NODE;                };
template <> struct PrismNodeTypeHelper<pm_rescue_modifier_node_t                 > { static constexpr pm_node_type_t TypeID = PM_RESCUE_MODIFIER_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_rescue_node_t                          > { static constexpr pm_node_type_t TypeID = PM_RESCUE_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_rest_parameter_node_t                  > { static constexpr pm_node_type_t TypeID = PM_REST_PARAMETER_NODE;                    };
template <> struct PrismNodeTypeHelper<pm_retry_node_t                           > { static constexpr pm_node_type_t TypeID = PM_RETRY_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_return_node_t                          > { static constexpr pm_node_type_t TypeID = PM_RETURN_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_self_node_t                            > { static constexpr pm_node_type_t TypeID = PM_SELF_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_shareable_constant_node_t              > { static constexpr pm_node_type_t TypeID = PM_SHAREABLE_CONSTANT_NODE;                };
template <> struct PrismNodeTypeHelper<pm_singleton_class_node_t                 > { static constexpr pm_node_type_t TypeID = PM_SINGLETON_CLASS_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_source_encoding_node_t                 > { static constexpr pm_node_type_t TypeID = PM_SOURCE_ENCODING_NODE;                   };
template <> struct PrismNodeTypeHelper<pm_source_file_node_t                     > { static constexpr pm_node_type_t TypeID = PM_SOURCE_FILE_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_source_line_node_t                     > { static constexpr pm_node_type_t TypeID = PM_SOURCE_LINE_NODE;                       };
template <> struct PrismNodeTypeHelper<pm_splat_node_t                           > { static constexpr pm_node_type_t TypeID = PM_SPLAT_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_statements_node_t                      > { static constexpr pm_node_type_t TypeID = PM_STATEMENTS_NODE;                        };
template <> struct PrismNodeTypeHelper<pm_string_node_t                          > { static constexpr pm_node_type_t TypeID = PM_STRING_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_super_node_t                           > { static constexpr pm_node_type_t TypeID = PM_SUPER_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_symbol_node_t                          > { static constexpr pm_node_type_t TypeID = PM_SYMBOL_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_true_node_t                            > { static constexpr pm_node_type_t TypeID = PM_TRUE_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_undef_node_t                           > { static constexpr pm_node_type_t TypeID = PM_UNDEF_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_unless_node_t                          > { static constexpr pm_node_type_t TypeID = PM_UNLESS_NODE;                            };
template <> struct PrismNodeTypeHelper<pm_until_node_t                           > { static constexpr pm_node_type_t TypeID = PM_UNTIL_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_when_node_t                            > { static constexpr pm_node_type_t TypeID = PM_WHEN_NODE;                              };
template <> struct PrismNodeTypeHelper<pm_while_node_t                           > { static constexpr pm_node_type_t TypeID = PM_WHILE_NODE;                             };
template <> struct PrismNodeTypeHelper<pm_x_string_node_t                        > { static constexpr pm_node_type_t TypeID = PM_X_STRING_NODE;                          };
template <> struct PrismNodeTypeHelper<pm_yield_node_t                           > { static constexpr pm_node_type_t TypeID = PM_YIELD_NODE;                             };
// clang-format on

using std::is_same_v;

// Returns true if the given `T` is a Prism node types. All Prism node types start with a `pm_node_t base` member.
template <typename T> constexpr bool isPrismNode = is_same_v<decltype(T::base), pm_node_t>;

// Take a pointer to a Prism node "subclass" (a thing with an embedded `pm_node_t base` as its first member),
// and up-casts it back to a general `pm_node_t` pointer.
template <typename PrismNode> pm_node_t *up_cast(PrismNode *node) {
    static_assert(!is_same_v<PrismNode, pm_node_t>,
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

} // namespace sorbet::parser::Prism

#endif // SORBET_PARSER_PRISM_HELPERS_H
