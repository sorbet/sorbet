#ifndef RUBY_PARSER_BUILDER_HH
#define RUBY_PARSER_BUILDER_HH

#include <memory>
#include <type_traits>
#include <vector>

namespace ruby_parser {

using ForeignPtr = const void *;
using SelfPtr = const void *;

struct node_list;
struct token;

struct builder {
    ForeignPtr (*accessible)(SelfPtr builder, ForeignPtr node);
    ForeignPtr (*alias)(SelfPtr builder, const token *alias, ForeignPtr to, ForeignPtr from);
    ForeignPtr (*arg)(SelfPtr builder, const token *name);
    ForeignPtr (*args)(SelfPtr builder, const token *begin, const node_list *args, const token *end, bool check_args);
    ForeignPtr (*array)(SelfPtr builder, const token *begin, const node_list *elements, const token *end);
    ForeignPtr (*array_pattern)(SelfPtr builder, const token *begin, const node_list *elements, const token *end);
    ForeignPtr (*assign)(SelfPtr builder, ForeignPtr lhs, const token *eql, ForeignPtr rhs);
    ForeignPtr (*assignable)(SelfPtr builder, ForeignPtr node);
    ForeignPtr (*associate)(SelfPtr builder, const token *begin, const node_list *pairs, const token *end);
    ForeignPtr (*assoc_error)(SelfPtr builder, const token *label, const token *fcall);
    ForeignPtr (*attrAsgn)(SelfPtr builder, ForeignPtr receiver, const token *dot, const token *selector, bool masgn);
    ForeignPtr (*backRef)(SelfPtr builder, const token *tok);
    ForeignPtr (*begin)(SelfPtr builder, const token *begin, ForeignPtr body, const token *end);
    ForeignPtr (*beginBody)(SelfPtr builder, ForeignPtr body, const node_list *rescueBodies, const token *elseTok,
                            ForeignPtr else_, const token *ensure_tok, ForeignPtr ensure);
    ForeignPtr (*beginKeyword)(SelfPtr builder, const token *begin, ForeignPtr body, const token *end);
    ForeignPtr (*binaryOp)(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg);
    ForeignPtr (*block)(SelfPtr builder, ForeignPtr methodCall, const token *begin, ForeignPtr args, ForeignPtr body,
                        const token *end);
    ForeignPtr (*blockPass)(SelfPtr builder, const token *amper, ForeignPtr arg);
    ForeignPtr (*blockarg)(SelfPtr builder, const token *amper, const token *name);
    ForeignPtr (*callLambda)(SelfPtr builder, const token *lambda);
    ForeignPtr (*call_method)(SelfPtr builder, ForeignPtr receiver, const token *dot, const token *selector,
                              const token *lparen, const node_list *args, const token *rparen);
    ForeignPtr (*call_method_error)(SelfPtr builder, ForeignPtr receiver, const token *dot);
    ForeignPtr (*case_)(SelfPtr builder, const token *case_, ForeignPtr expr, const node_list *whenBodies,
                        const token *elseTok, ForeignPtr elseBody, const token *end);
    ForeignPtr (*case_error)(SelfPtr builder, const token *case_, ForeignPtr cond, const token *end);
    ForeignPtr (*case_match)(SelfPtr builder, const token *case_, ForeignPtr expr, const node_list *inBodies,
                             const token *elseTok, ForeignPtr elseBody, const token *end);
    ForeignPtr (*character)(SelfPtr builder, const token *char_);
    ForeignPtr (*complex)(SelfPtr builder, const token *tok);
    ForeignPtr (*compstmt)(SelfPtr builder, const node_list *node);
    ForeignPtr (*condition)(SelfPtr builder, const token *cond_tok, ForeignPtr cond, const token *then,
                            ForeignPtr ifTrue, const token *else_, ForeignPtr ifFalse, const token *end);
    ForeignPtr (*conditionMod)(SelfPtr builder, ForeignPtr ifTrue, ForeignPtr ifFalse, ForeignPtr cond);
    ForeignPtr (*const_)(SelfPtr builder, const token *name);
    ForeignPtr (*const_pattern)(SelfPtr builder, ForeignPtr const_, const token *begin, ForeignPtr pattern,
                                const token *end);
    ForeignPtr (*constFetch)(SelfPtr builder, ForeignPtr scope, const token *colon, const token *name);
    ForeignPtr (*constFetchError)(SelfPtr builder, ForeignPtr scope, const token *colon);
    ForeignPtr (*constGlobal)(SelfPtr builder, const token *colon, const token *name);
    ForeignPtr (*constOpAssignable)(SelfPtr builder, ForeignPtr node);
    ForeignPtr (*cvar)(SelfPtr builder, const token *tok);
    ForeignPtr (*dedentString)(SelfPtr builder, ForeignPtr node, size_t dedentLevel);
    ForeignPtr (*def_class)(SelfPtr builder, const token *class_, ForeignPtr name, const token *lt_,
                            ForeignPtr superclass, ForeignPtr body, const token *end_);
    ForeignPtr (*defEndlessMethod)(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, const token *equal,
                                   ForeignPtr body);
    ForeignPtr (*defEndlessSingleton)(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, const token *equal,
                                      ForeignPtr body);
    ForeignPtr (*defMethod)(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, ForeignPtr body, const token *end);
    ForeignPtr (*defModule)(SelfPtr builder, const token *module, ForeignPtr name, ForeignPtr body, const token *end_);
    ForeignPtr (*defnHead)(SelfPtr builder, const token *def, const token *name);
    ForeignPtr (*defnHeadError)(SelfPtr builder, const token *def);
    ForeignPtr (*def_sclass)(SelfPtr builder, const token *class_, const token *lshft_, ForeignPtr expr,
                             ForeignPtr body, const token *end_);
    ForeignPtr (*defsHead)(SelfPtr builder, const token *def, ForeignPtr definee, const token *dot, const token *name);
    ForeignPtr (*defsHeadError)(SelfPtr builder, const token *def, ForeignPtr definee, const token *dot);
    ForeignPtr (*defSingleton)(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, ForeignPtr body, const token *end);
    ForeignPtr (*encodingLiteral)(SelfPtr builder, const token *tok);
    ForeignPtr (*error_node)(SelfPtr builder, size_t begin, size_t end);
    ForeignPtr (*false_)(SelfPtr builder, const token *tok);
    ForeignPtr (*find_pattern)(SelfPtr builder, const token *lbrack_t, const node_list *elements,
                               const token *rbrack_t);
    ForeignPtr (*fileLiteral)(SelfPtr builder, const token *tok);
    ForeignPtr (*float_)(SelfPtr builder, const token *tok);
    ForeignPtr (*floatComplex)(SelfPtr builder, const token *tok);
    ForeignPtr (*for_)(SelfPtr builder, const token *for_, ForeignPtr iterator, const token *in_, ForeignPtr iteratee,
                       const token *do_, ForeignPtr body, const token *end);
    ForeignPtr (*forward_arg)(SelfPtr builder, const token *begin, const token *dots, const token *end);
    ForeignPtr (*forwarded_args)(SelfPtr builder, const token *dots);
    ForeignPtr (*forwarded_restarg)(SelfPtr builder, const token *star);
    ForeignPtr (*forwarded_kwrestarg)(SelfPtr builder, const token *dstar);
    ForeignPtr (*gvar)(SelfPtr builder, const token *tok);
    ForeignPtr (*hash_pattern)(SelfPtr builder, const token *begin, const node_list *kwargs, const token *end);
    ForeignPtr (*ident)(SelfPtr builder, const token *tok);
    ForeignPtr (*if_guard)(SelfPtr builder, const token *tok, ForeignPtr ifBody);
    ForeignPtr (*in_pattern)(SelfPtr builders, const token *tok, ForeignPtr pattern, ForeignPtr guard,
                             const token *thenToken, ForeignPtr body);
    ForeignPtr (*index)(SelfPtr builder, ForeignPtr receiver, const token *lbrack, const node_list *indexes,
                        const token *rbrack);
    ForeignPtr (*indexAsgn)(SelfPtr builder, ForeignPtr receiver, const token *lbrack, const node_list *indexes,
                            const token *rbrack);
    ForeignPtr (*integer)(SelfPtr builder, const token *tok);
    ForeignPtr (*ivar)(SelfPtr builder, const token *tok);
    ForeignPtr (*keywordBreak)(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                               const token *rparen);
    ForeignPtr (*keywordDefined)(SelfPtr builder, const token *keyword, ForeignPtr arg);
    ForeignPtr (*keywordNext)(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                              const token *rparen);
    ForeignPtr (*keywordRedo)(SelfPtr builder, const token *keyword);
    ForeignPtr (*keywordRetry)(SelfPtr builder, const token *keyword);
    ForeignPtr (*keywordReturn)(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                                const token *rparen);
    ForeignPtr (*keywordSuper)(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                               const token *rparen);
    ForeignPtr (*keywordYield)(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                               const token *rparen);
    ForeignPtr (*keywordZsuper)(SelfPtr builder, const token *keyword);
    ForeignPtr (*kwarg)(SelfPtr builder, const token *name);
    ForeignPtr (*kwoptarg)(SelfPtr builder, const token *name, ForeignPtr value);
    ForeignPtr (*kwnilarg)(SelfPtr builder, ForeignPtr node);
    ForeignPtr (*kwrestarg)(SelfPtr builder, const token *dstar, const token *name);
    ForeignPtr (*kwsplat)(SelfPtr builder, const token *dstar, ForeignPtr arg);
    ForeignPtr (*line_literal)(SelfPtr builder, const token *tok);
    ForeignPtr (*logicalAnd)(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs);
    ForeignPtr (*logicalOr)(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs);
    ForeignPtr (*loopUntil)(SelfPtr builder, const token *keyword, ForeignPtr cond, const token *do_, ForeignPtr body,
                            const token *end);
    ForeignPtr (*loopUntil_mod)(SelfPtr builder, ForeignPtr body, ForeignPtr cond);
    ForeignPtr (*loop_while)(SelfPtr builder, const token *keyword, ForeignPtr cond, const token *do_, ForeignPtr body,
                             const token *end);
    ForeignPtr (*loop_while_mod)(SelfPtr builder, ForeignPtr body, ForeignPtr cond);
    ForeignPtr (*match_alt)(SelfPtr builder, ForeignPtr left, const token *pipe, ForeignPtr right);
    ForeignPtr (*match_as)(SelfPtr builder, ForeignPtr value, const token *assoc, ForeignPtr as);
    ForeignPtr (*match_label)(SelfPtr builder, ForeignPtr label);
    ForeignPtr (*match_pattern)(SelfPtr builder, ForeignPtr lhs, const token *tok, ForeignPtr rhs);
    ForeignPtr (*match_pattern_p)(SelfPtr builder, ForeignPtr lhs, const token *tok, ForeignPtr rhs);
    ForeignPtr (*match_nil_pattern)(SelfPtr builder, const token *dstar, const token *nil);
    ForeignPtr (*match_op)(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg);
    ForeignPtr (*match_pair)(SelfPtr builder, ForeignPtr label, ForeignPtr value);
    ForeignPtr (*match_rest)(SelfPtr builder, const token *star, const token *name);
    ForeignPtr (*match_var)(SelfPtr builder, const token *name);
    ForeignPtr (*match_with_trailing_comma)(SelfPtr builder, ForeignPtr match);
    ForeignPtr (*multi_assign)(SelfPtr builder, ForeignPtr mlhs, ForeignPtr rhs);
    ForeignPtr (*multi_lhs)(SelfPtr builder, const token *begin, const node_list *items, const token *end);
    ForeignPtr (*multi_lhs1)(SelfPtr builder, const token *begin, ForeignPtr item, const token *end);
    ForeignPtr (*nil)(SelfPtr builder, const token *tok);
    ForeignPtr (*not_op)(SelfPtr builder, const token *not_, const token *begin, ForeignPtr receiver, const token *end);
    ForeignPtr (*nth_ref)(SelfPtr builder, const token *tok);
    ForeignPtr (*numparams)(SelfPtr builder, const node_list *declaringNodes);
    ForeignPtr (*numblock)(SelfPtr builder, ForeignPtr methodCall, const token *begin, ForeignPtr args, ForeignPtr body,
                           const token *end);
    ForeignPtr (*op_assign)(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs);
    ForeignPtr (*optarg)(SelfPtr builder, const token *name, const token *eql, ForeignPtr value);
    ForeignPtr (*p_ident)(SelfPtr builder, const token *tok);
    ForeignPtr (*pair)(SelfPtr builder, ForeignPtr key, const token *assoc, ForeignPtr value);
    ForeignPtr (*pair_keyword)(SelfPtr builder, const token *key, ForeignPtr value);
    ForeignPtr (*pair_label)(SelfPtr builder, const token *key);
    ForeignPtr (*pair_quoted)(SelfPtr builder, const token *begin, const node_list *parts, const token *end,
                              ForeignPtr value);
    ForeignPtr (*pin)(SelfPtr builder, const token *tok, ForeignPtr var);
    ForeignPtr (*postexe)(SelfPtr builder, const token *begin, ForeignPtr node, const token *rbrace);
    ForeignPtr (*preexe)(SelfPtr builder, const token *begin, ForeignPtr node, const token *rbrace);
    ForeignPtr (*procarg0)(SelfPtr builder, ForeignPtr arg);
    ForeignPtr (*range_exclusive)(SelfPtr builder, ForeignPtr lhs, const token *oper, ForeignPtr rhs);
    ForeignPtr (*range_inclusive)(SelfPtr builder, ForeignPtr lhs, const token *oper, ForeignPtr rhs);
    ForeignPtr (*rational)(SelfPtr builder, const token *tok);
    ForeignPtr (*rational_complex)(SelfPtr builder, const token *tok);
    ForeignPtr (*regexp_compose)(SelfPtr builder, const token *begin, const node_list *parts, const token *end,
                                 ForeignPtr options);
    ForeignPtr (*regexp_options)(SelfPtr builder, const token *regopt);
    ForeignPtr (*rescue_body)(SelfPtr builder, const token *rescue, ForeignPtr excList, const token *assoc,
                              ForeignPtr excVar, const token *then, ForeignPtr body);
    ForeignPtr (*restarg)(SelfPtr builder, const token *star, const token *name);
    ForeignPtr (*self_)(SelfPtr builder, const token *tok);
    ForeignPtr (*shadowarg)(SelfPtr builder, const token *name);
    ForeignPtr (*splat)(SelfPtr builder, const token *star, ForeignPtr arg);
    ForeignPtr (*splat_mlhs)(SelfPtr builder, const token *star, ForeignPtr arg);
    ForeignPtr (*string)(SelfPtr builder, const token *string_);
    ForeignPtr (*string_compose)(SelfPtr builder, const token *begin, const node_list *parts, const token *end);
    ForeignPtr (*string_internal)(SelfPtr builder, const token *string_);
    ForeignPtr (*symbol)(SelfPtr builder, const token *symbol);
    ForeignPtr (*symbol_compose)(SelfPtr builder, const token *begin, const node_list *parts, const token *end);
    ForeignPtr (*symbol_internal)(SelfPtr builder, const token *symbol);
    ForeignPtr (*symbols_compose)(SelfPtr builder, const token *begin, const node_list *parts, const token *end);
    ForeignPtr (*ternary)(SelfPtr builder, ForeignPtr cond, const token *question, ForeignPtr ifTrue,
                          const token *colon, ForeignPtr ifFalse);
    ForeignPtr (*true_)(SelfPtr builder, const token *tok);
    ForeignPtr (*truncateBodyStmt)(SelfPtr builder, ForeignPtr body, const token *truncateTok);
    ForeignPtr (*unary_op)(SelfPtr builder, const token *oper, ForeignPtr receiver);
    ForeignPtr (*undefMethod)(SelfPtr builder, const token *undef, const node_list *name_list);
    ForeignPtr (*unless_guard)(SelfPtr builder, const token *unlessGuard, ForeignPtr unlessBody);
    ForeignPtr (*when)(SelfPtr builder, const token *when, const node_list *patterns, const token *then,
                       ForeignPtr body);
    ForeignPtr (*word)(SelfPtr builder, const node_list *parts);
    ForeignPtr (*words_compose)(SelfPtr builder, const token *begin, const node_list *parts, const token *end);
    ForeignPtr (*xstring_compose)(SelfPtr builder, const token *begin, const node_list *parts, const token *end);
};

static_assert(std::is_pod<builder>::value, "`builder` must be a POD type");

} // namespace ruby_parser

#endif
