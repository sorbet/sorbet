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

}

namespace sorbet {
namespace parser {

using namespace ruby_parser;

struct BuilderForwarder {
    ForeignPtr accessible(SelfPtr builder, ForeignPtr node) const;
    ForeignPtr alias(SelfPtr builder, const token *alias, ForeignPtr to, ForeignPtr from) const;
    ForeignPtr arg(SelfPtr builder, const token *name) const;
    ForeignPtr args(SelfPtr builder, const token *begin, const node_list *args, const token *end, bool check_args) const;
    ForeignPtr array(SelfPtr builder, const token *begin, const node_list *elements, const token *end) const;
    ForeignPtr array_pattern(SelfPtr builder, const token *begin, const node_list *elements, const token *end) const;
    ForeignPtr assign(SelfPtr builder, ForeignPtr lhs, const token *eql, ForeignPtr rhs) const;
    ForeignPtr assignable(SelfPtr builder, ForeignPtr node) const;
    ForeignPtr associate(SelfPtr builder, const token *begin, const node_list *pairs, const token *end) const;
    ForeignPtr attrAsgn(SelfPtr builder, ForeignPtr receiver, const token *dot, const token *selector, bool masgn) const;
    ForeignPtr backRef(SelfPtr builder, const token *tok) const;
    ForeignPtr begin(SelfPtr builder, const token *begin, ForeignPtr body, const token *end) const;
    ForeignPtr beginBody(SelfPtr builder, ForeignPtr body, const node_list *rescueBodies, const token *elseTok,
                            ForeignPtr else_, const token *ensure_tok, ForeignPtr ensure) const;
    ForeignPtr beginKeyword(SelfPtr builder, const token *begin, ForeignPtr body, const token *end) const;
    ForeignPtr binaryOp(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg) const;
    ForeignPtr block(SelfPtr builder, ForeignPtr methodCall, const token *begin, ForeignPtr args, ForeignPtr body,
                        const token *end) const;
    ForeignPtr blockPass(SelfPtr builder, const token *amper, ForeignPtr arg) const;
    ForeignPtr blockarg(SelfPtr builder, const token *amper, const token *name) const;
    ForeignPtr callLambda(SelfPtr builder, const token *lambda) const;
    ForeignPtr call_method(SelfPtr builder, ForeignPtr receiver, const token *dot, const token *selector,
                              const token *lparen, const node_list *args, const token *rparen) const;
    ForeignPtr case_(SelfPtr builder, const token *case_, ForeignPtr expr, const node_list *whenBodies,
                        const token *elseTok, ForeignPtr elseBody, const token *end) const;
    ForeignPtr case_match(SelfPtr builder, const token *case_, ForeignPtr expr, const node_list *inBodies,
                             const token *elseTok, ForeignPtr elseBody, const token *end) const;
    ForeignPtr character(SelfPtr builder, const token *char_) const;
    ForeignPtr complex(SelfPtr builder, const token *tok) const;
    ForeignPtr compstmt(SelfPtr builder, const node_list *node) const;
    ForeignPtr condition(SelfPtr builder, const token *cond_tok, ForeignPtr cond, const token *then,
                            ForeignPtr ifTrue, const token *else_, ForeignPtr ifFalse, const token *end) const;
    ForeignPtr conditionMod(SelfPtr builder, ForeignPtr ifTrue, ForeignPtr ifFalse, ForeignPtr cond) const;
    ForeignPtr const_(SelfPtr builder, const token *name) const;
    ForeignPtr const_pattern(SelfPtr builder, ForeignPtr const_, const token *begin, ForeignPtr pattern,
                                const token *end) const;
    ForeignPtr constFetch(SelfPtr builder, ForeignPtr scope, const token *colon, const token *name) const;
    ForeignPtr constGlobal(SelfPtr builder, const token *colon, const token *name) const;
    ForeignPtr constOpAssignable(SelfPtr builder, ForeignPtr node) const;
    ForeignPtr cvar(SelfPtr builder, const token *tok) const;
    ForeignPtr dedentString(SelfPtr builder, ForeignPtr node, size_t dedentLevel) const;
    ForeignPtr def_class(SelfPtr builder, const token *class_, ForeignPtr name, const token *lt_,
                            ForeignPtr superclass, ForeignPtr body, const token *end_) const;
    ForeignPtr defEndlessMethod(SelfPtr builder, const token *def, const token *name, ForeignPtr args,
                                   const token *equal, ForeignPtr body) const;
    ForeignPtr defEndlessSingleton(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, const token *equal,
                                      ForeignPtr body) const;
    ForeignPtr defMethod(SelfPtr builder, const token *def, const token *name, ForeignPtr args, ForeignPtr body,
                            const token *end) const;
    ForeignPtr defModule(SelfPtr builder, const token *module, ForeignPtr name, ForeignPtr body, const token *end_) const;
    ForeignPtr def_sclass(SelfPtr builder, const token *class_, const token *lshft_, ForeignPtr expr,
                             ForeignPtr body, const token *end_) const;
    ForeignPtr defsHead(SelfPtr builder, const token *def, ForeignPtr definee, const token *dot, const token *name) const;
    ForeignPtr defSingleton(SelfPtr builder, ForeignPtr defHead, ForeignPtr args, ForeignPtr body, const token *end) const;
    ForeignPtr encodingLiteral(SelfPtr builder, const token *tok) const;
    ForeignPtr false_(SelfPtr builder, const token *tok) const;
    ForeignPtr find_pattern(SelfPtr builder, const token *lbrack_t, const node_list *elements,
                               const token *rbrack_t) const;
    ForeignPtr fileLiteral(SelfPtr builder, const token *tok) const;
    ForeignPtr float_(SelfPtr builder, const token *tok) const;
    ForeignPtr floatComplex(SelfPtr builder, const token *tok) const;
    ForeignPtr for_(SelfPtr builder, const token *for_, ForeignPtr iterator, const token *in_, ForeignPtr iteratee,
                       const token *do_, ForeignPtr body, const token *end) const;
    ForeignPtr forward_arg(SelfPtr builder, const token *begin, const token *dots, const token *end) const;
    ForeignPtr forwarded_args(SelfPtr builder, const token *dots) const;
    ForeignPtr gvar(SelfPtr builder, const token *tok) const;
    ForeignPtr hash_pattern(SelfPtr builder, const token *begin, const node_list *kwargs, const token *end) const;
    ForeignPtr ident(SelfPtr builder, const token *tok) const;
    ForeignPtr if_guard(SelfPtr builder, const token *tok, ForeignPtr ifBody) const;
    ForeignPtr in_pattern(SelfPtr builders, const token *tok, ForeignPtr pattern, ForeignPtr guard,
                             const token *thenToken, ForeignPtr body) const;
    ForeignPtr index(SelfPtr builder, ForeignPtr receiver, const token *lbrack, const node_list *indexes,
                        const token *rbrack) const;
    ForeignPtr indexAsgn(SelfPtr builder, ForeignPtr receiver, const token *lbrack, const node_list *indexes,
                            const token *rbrack) const;
    ForeignPtr integer(SelfPtr builder, const token *tok) const;
    ForeignPtr ivar(SelfPtr builder, const token *tok) const;
    ForeignPtr keywordBreak(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                               const token *rparen) const;
    ForeignPtr keywordDefined(SelfPtr builder, const token *keyword, ForeignPtr arg) const;
    ForeignPtr keywordNext(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                              const token *rparen) const;
    ForeignPtr keywordRedo(SelfPtr builder, const token *keyword) const;
    ForeignPtr keywordRetry(SelfPtr builder, const token *keyword) const;
    ForeignPtr keywordReturn(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                                const token *rparen) const;
    ForeignPtr keywordSuper(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                               const token *rparen) const;
    ForeignPtr keywordYield(SelfPtr builder, const token *keyword, const token *lparen, const node_list *args,
                               const token *rparen) const;
    ForeignPtr keywordZsuper(SelfPtr builder, const token *keyword) const;
    ForeignPtr kwarg(SelfPtr builder, const token *name) const;
    ForeignPtr kwoptarg(SelfPtr builder, const token *name, ForeignPtr value) const;
    ForeignPtr kwnilarg(SelfPtr builder, const token *dstar, const token *nil) const;
    ForeignPtr kwrestarg(SelfPtr builder, const token *dstar, const token *name) const;
    ForeignPtr kwsplat(SelfPtr builder, const token *dstar, ForeignPtr arg) const;
    ForeignPtr line_literal(SelfPtr builder, const token *tok) const;
    ForeignPtr logicalAnd(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs) const;
    ForeignPtr logicalOr(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs) const;
    ForeignPtr loopUntil(SelfPtr builder, const token *keyword, ForeignPtr cond, const token *do_, ForeignPtr body,
                            const token *end) const;
    ForeignPtr loopUntil_mod(SelfPtr builder, ForeignPtr body, ForeignPtr cond) const;
    ForeignPtr loop_while(SelfPtr builder, const token *keyword, ForeignPtr cond, const token *do_, ForeignPtr body,
                             const token *end) const;
    ForeignPtr loop_while_mod(SelfPtr builder, ForeignPtr body, ForeignPtr cond) const;
    ForeignPtr match_alt(SelfPtr builder, ForeignPtr left, const token *pipe, ForeignPtr right) const;
    ForeignPtr match_as(SelfPtr builder, ForeignPtr value, const token *assoc, ForeignPtr as) const;
    ForeignPtr match_label(SelfPtr builder, ForeignPtr label) const;
    ForeignPtr match_pattern(SelfPtr builder, ForeignPtr lhs, const token *tok, ForeignPtr rhs) const;
    ForeignPtr match_pattern_p(SelfPtr builder, ForeignPtr lhs, const token *tok, ForeignPtr rhs) const;
    ForeignPtr match_nil_pattern(SelfPtr builder, const token *dstar, const token *nil) const;
    ForeignPtr match_op(SelfPtr builder, ForeignPtr receiver, const token *oper, ForeignPtr arg) const;
    ForeignPtr match_pair(SelfPtr builder, ForeignPtr label, ForeignPtr value) const;
    ForeignPtr match_rest(SelfPtr builder, const token *star, const token *name) const;
    ForeignPtr match_var(SelfPtr builder, const token *name) const;
    ForeignPtr match_with_trailing_comma(SelfPtr builder, ForeignPtr match) const;
    ForeignPtr multi_assign(SelfPtr builder, ForeignPtr mlhs, ForeignPtr rhs) const;
    ForeignPtr multi_lhs(SelfPtr builder, const token *begin, const node_list *items, const token *end) const;
    ForeignPtr multi_lhs1(SelfPtr builder, const token *begin, ForeignPtr item, const token *end) const;
    ForeignPtr nil(SelfPtr builder, const token *tok) const;
    ForeignPtr not_op(SelfPtr builder, const token *not_, const token *begin, ForeignPtr receiver, const token *end) const;
    ForeignPtr nth_ref(SelfPtr builder, const token *tok) const;
    ForeignPtr numparams(SelfPtr builder, const node_list *declaringNodes) const;
    ForeignPtr numblock(SelfPtr builder, ForeignPtr methodCall, const token *begin, ForeignPtr args, ForeignPtr body,
                           const token *end) const;
    ForeignPtr op_assign(SelfPtr builder, ForeignPtr lhs, const token *op, ForeignPtr rhs) const;
    ForeignPtr optarg(SelfPtr builder, const token *name, const token *eql, ForeignPtr value) const;
    ForeignPtr p_ident(SelfPtr builder, const token *tok) const;
    ForeignPtr pair(SelfPtr builder, ForeignPtr key, const token *assoc, ForeignPtr value) const;
    ForeignPtr pair_keyword(SelfPtr builder, const token *key, ForeignPtr value) const;
    ForeignPtr pair_quoted(SelfPtr builder, const token *begin, const node_list *parts, const token *end,
                              ForeignPtr value) const;
    ForeignPtr pin(SelfPtr builder, const token *tok, ForeignPtr var) const;
    ForeignPtr postexe(SelfPtr builder, const token *begin, ForeignPtr node, const token *rbrace) const;
    ForeignPtr preexe(SelfPtr builder, const token *begin, ForeignPtr node, const token *rbrace) const;
    ForeignPtr procarg0(SelfPtr builder, ForeignPtr arg) const;
    ForeignPtr range_exclusive(SelfPtr builder, ForeignPtr lhs, const token *oper, ForeignPtr rhs) const;
    ForeignPtr range_inclusive(SelfPtr builder, ForeignPtr lhs, const token *oper, ForeignPtr rhs) const;
    ForeignPtr rational(SelfPtr builder, const token *tok) const;
    ForeignPtr rational_complex(SelfPtr builder, const token *tok) const;
    ForeignPtr regexp_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end,
                                 ForeignPtr options) const;
    ForeignPtr regexp_options(SelfPtr builder, const token *regopt) const;
    ForeignPtr rescue_body(SelfPtr builder, const token *rescue, ForeignPtr excList, const token *assoc,
                              ForeignPtr excVar, const token *then, ForeignPtr body) const;
    ForeignPtr restarg(SelfPtr builder, const token *star, const token *name) const;
    ForeignPtr self_(SelfPtr builder, const token *tok) const;
    ForeignPtr shadowarg(SelfPtr builder, const token *name) const;
    ForeignPtr splat(SelfPtr builder, const token *star, ForeignPtr arg) const;
    ForeignPtr splat_mlhs(SelfPtr builder, const token *star, ForeignPtr arg) const;
    ForeignPtr string(SelfPtr builder, const token *string_) const;
    ForeignPtr string_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) const;
    ForeignPtr string_internal(SelfPtr builder, const token *string_) const;
    ForeignPtr symbol(SelfPtr builder, const token *symbol) const;
    ForeignPtr symbol_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) const;
    ForeignPtr symbol_internal(SelfPtr builder, const token *symbol) const;
    ForeignPtr symbols_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) const;
    ForeignPtr ternary(SelfPtr builder, ForeignPtr cond, const token *question, ForeignPtr ifTrue,
                          const token *colon, ForeignPtr ifFalse) const;
    ForeignPtr true_(SelfPtr builder, const token *tok) const;
    ForeignPtr unary_op(SelfPtr builder, const token *oper, ForeignPtr receiver) const;
    ForeignPtr undefMethod(SelfPtr builder, const token *undef, const node_list *name_list) const;
    ForeignPtr unless_guard(SelfPtr builder, const token *unlessGuard, ForeignPtr unlessBody) const;
    ForeignPtr when(SelfPtr builder, const token *when, const node_list *patterns, const token *then,
                       ForeignPtr body) const;
    ForeignPtr word(SelfPtr builder, const node_list *parts) const;
    ForeignPtr words_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) const;
    ForeignPtr xstring_compose(SelfPtr builder, const token *begin, const node_list *parts, const token *end) const;
};

}
}

#endif
