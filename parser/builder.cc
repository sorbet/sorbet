#include "ruby_parser/builder.hh"

namespace sruby {
namespace parser {

using ruby_parser::foreign_ptr;
using ruby_parser::self_ptr;
using ruby_parser::token;
using ruby_parser::node_list;

foreign_ptr accessible(self_ptr builder, foreign_ptr node) { return nullptr; }

foreign_ptr alias(self_ptr builder, const token *alias, foreign_ptr to,
                  foreign_ptr from) {
  return nullptr;
}

foreign_ptr arg(self_ptr builder, const token *name) { return nullptr; }

foreign_ptr args(self_ptr builder, const token *begin, const node_list *args,
                 const token *end, bool check_args) {
  return nullptr;
}

foreign_ptr array(self_ptr builder, const token *begin,
                  const node_list *elements, const token *end) {
  return nullptr;
}

foreign_ptr assign(self_ptr builder, foreign_ptr lhs, const token *eql,
                   foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr assignable(self_ptr builder, foreign_ptr node) { return nullptr; }

foreign_ptr associate(self_ptr builder, const token *begin,
                      const node_list *pairs, const token *end) {
  return nullptr;
}

foreign_ptr attr_asgn(self_ptr builder, foreign_ptr receiver, const token *dot,
                      const token *selector) {
  return nullptr;
}

foreign_ptr back_ref(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr begin(self_ptr builder, const token *begin, foreign_ptr body,
                  const token *end) {
  return nullptr;
}

foreign_ptr begin_body(self_ptr builder, foreign_ptr body,
                       const node_list *rescue_bodies, const token *else_tok,
                       foreign_ptr else_, const token *ensure_tok,
                       foreign_ptr ensure) {
  return nullptr;
}

foreign_ptr begin_keyword(self_ptr builder, const token *begin,
                          foreign_ptr body, const token *end) {
  return nullptr;
}

foreign_ptr binary_op(self_ptr builder, foreign_ptr receiver, const token *oper,
                      foreign_ptr arg) {
  return nullptr;
}

foreign_ptr block(self_ptr builder, foreign_ptr method_call, const token *begin,
                  foreign_ptr args, foreign_ptr body, const token *end) {
  return nullptr;
}

foreign_ptr block_pass(self_ptr builder, const token *amper, foreign_ptr arg) {
  return nullptr;
}

foreign_ptr blockarg(self_ptr builder, const token *amper, const token *name) {
  return nullptr;
}

foreign_ptr call_lambda(self_ptr builder, const token *lambda) {
  return nullptr;
}

foreign_ptr call_method(self_ptr builder, foreign_ptr receiver,
                        const token *dot, const token *selector,
                        const token *lparen, const node_list *args,
                        const token *rparen) {
  return nullptr;
}

foreign_ptr case_(self_ptr builder, const token *case_, foreign_ptr expr,
                  const node_list *when_bodies, const token *else_tok,
                  foreign_ptr else_body, const token *end) {
  return nullptr;
}

foreign_ptr character(self_ptr builder, const token *char_) { return nullptr; }

foreign_ptr complex(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr compstmt(self_ptr builder, const node_list *node) {
  return nullptr;
}

foreign_ptr condition(self_ptr builder, const token *cond_tok, foreign_ptr cond,
                      const token *then, foreign_ptr if_true,
                      const token *else_, foreign_ptr if_false,
                      const token *end) {
  return nullptr;
}

foreign_ptr condition_mod(self_ptr builder, foreign_ptr if_true,
                          foreign_ptr if_false, foreign_ptr cond) {
  return nullptr;
}

foreign_ptr const_(self_ptr builder, const token *name) { return nullptr; }

foreign_ptr const_fetch(self_ptr builder, foreign_ptr scope, const token *colon,
                        const token *name) {
  return nullptr;
}

foreign_ptr const_global(self_ptr builder, const token *colon,
                         const token *name) {
  return nullptr;
}

foreign_ptr const_op_assignable(self_ptr builder, foreign_ptr node) {
  return nullptr;
}

foreign_ptr cvar(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr dedent_string(self_ptr builder, foreign_ptr node,
                          size_t dedent_level) {
  return nullptr;
}

foreign_ptr def_class(self_ptr builder, const token *class_, foreign_ptr name,
                      const token *lt_, foreign_ptr superclass,
                      foreign_ptr body, const token *end_) {
  return nullptr;
}

foreign_ptr def_method(self_ptr builder, const token *def, const token *name,
                       foreign_ptr args, foreign_ptr body, const token *end) {
  return nullptr;
}

foreign_ptr def_module(self_ptr builder, const token *module, foreign_ptr name,
                       foreign_ptr body, const token *end_) {
  return nullptr;
}

foreign_ptr def_sclass(self_ptr builder, const token *class_,
                       const token *lshft_, foreign_ptr expr, foreign_ptr body,
                       const token *end_) {
  return nullptr;
}

foreign_ptr def_singleton(self_ptr builder, const token *def,
                          foreign_ptr definee, const token *dot,
                          const token *name, foreign_ptr args, foreign_ptr body,
                          const token *end) {
  return nullptr;
}

foreign_ptr encoding_literal(self_ptr builder, const token *tok) {
  return nullptr;
}

foreign_ptr false_(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr file_literal(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr float_(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr float_complex(self_ptr builder, const token *tok) {
  return nullptr;
}

foreign_ptr for_(self_ptr builder, const token *for_, foreign_ptr iterator,
                 const token *in_, foreign_ptr iteratee, const token *do_,
                 foreign_ptr body, const token *end) {
  return nullptr;
}

foreign_ptr gvar(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr ident(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr index(self_ptr builder, foreign_ptr receiver, const token *lbrack,
                  const node_list *indexes, const token *rbrack) {
  return nullptr;
}

foreign_ptr index_asgn(self_ptr builder, foreign_ptr receiver,
                       const token *lbrack, const node_list *indexes,
                       const token *rbrack) {
  return nullptr;
}

foreign_ptr integer(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr ivar(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr keyword_break(self_ptr builder, const token *keyword,
                          const token *lparen, const node_list *args,
                          const token *rparen) {
  return nullptr;
}

foreign_ptr keyword_defined(self_ptr builder, const token *keyword,
                            foreign_ptr arg) {
  return nullptr;
}

foreign_ptr keyword_next(self_ptr builder, const token *keyword,
                         const token *lparen, const node_list *args,
                         const token *rparen) {
  return nullptr;
}

foreign_ptr keyword_redo(self_ptr builder, const token *keyword) {
  return nullptr;
}

foreign_ptr keyword_retry(self_ptr builder, const token *keyword) {
  return nullptr;
}

foreign_ptr keyword_return(self_ptr builder, const token *keyword,
                           const token *lparen, const node_list *args,
                           const token *rparen) {
  return nullptr;
}

foreign_ptr keyword_super(self_ptr builder, const token *keyword,
                          const token *lparen, const node_list *args,
                          const token *rparen) {
  return nullptr;
}

foreign_ptr keyword_yield(self_ptr builder, const token *keyword,
                          const token *lparen, const node_list *args,
                          const token *rparen) {
  return nullptr;
}

foreign_ptr keyword_zsuper(self_ptr builder, const token *keyword) {
  return nullptr;
}

foreign_ptr kwarg(self_ptr builder, const token *name) { return nullptr; }

foreign_ptr kwoptarg(self_ptr builder, const token *name, foreign_ptr value) {
  return nullptr;
}

foreign_ptr kwrestarg(self_ptr builder, const token *dstar, const token *name) {
  return nullptr;
}

foreign_ptr kwsplat(self_ptr builder, const token *dstar, foreign_ptr arg) {
  return nullptr;
}

foreign_ptr line_literal(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr logical_and(self_ptr builder, foreign_ptr lhs, const token *op,
                        foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr logical_or(self_ptr builder, foreign_ptr lhs, const token *op,
                       foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr loop_until(self_ptr builder, const token *keyword, foreign_ptr cond,
                       const token *do_, foreign_ptr body, const token *end) {
  return nullptr;
}

foreign_ptr loop_until_mod(self_ptr builder, foreign_ptr body,
                           foreign_ptr cond) {
  return nullptr;
}

foreign_ptr loop_while(self_ptr builder, const token *keyword, foreign_ptr cond,
                       const token *do_, foreign_ptr body, const token *end) {
  return nullptr;
}

foreign_ptr loop_while_mod(self_ptr builder, foreign_ptr body,
                           foreign_ptr cond) {
  return nullptr;
}

foreign_ptr match_op(self_ptr builder, foreign_ptr receiver, const token *oper,
                     foreign_ptr arg) {
  return nullptr;
}

foreign_ptr multi_assign(self_ptr builder, foreign_ptr mlhs, foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr multi_lhs(self_ptr builder, const token *begin,
                      const node_list *items, const token *end) {
  return nullptr;
}

foreign_ptr multi_lhs1(self_ptr builder, const token *begin, foreign_ptr item,
                       const token *end) {
  return nullptr;
}

foreign_ptr negate(self_ptr builder, const token *uminus, foreign_ptr numeric) {
  return nullptr;
}

foreign_ptr nil(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr not_op(self_ptr builder, const token *not_, const token *begin,
                   foreign_ptr receiver, const token *end) {
  return nullptr;
}

foreign_ptr nth_ref(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr op_assign(self_ptr builder, foreign_ptr lhs, const token *op,
                      foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr optarg(self_ptr builder, const token *name, const token *eql,
                   foreign_ptr value) {
  return nullptr;
}

foreign_ptr pair(self_ptr builder, foreign_ptr key, const token *assoc,
                 foreign_ptr value) {
  return nullptr;
}

foreign_ptr pair_keyword(self_ptr builder, const token *key,
                         foreign_ptr value) {
  return nullptr;
}

foreign_ptr pair_quoted(self_ptr builder, const token *begin,
                        const node_list *parts, const token *end,
                        foreign_ptr value) {
  return nullptr;
}

foreign_ptr postexe(self_ptr builder, const token *begin, foreign_ptr node,
                    const token *rbrace) {
  return nullptr;
}

foreign_ptr preexe(self_ptr builder, const token *begin, foreign_ptr node,
                   const token *rbrace) {
  return nullptr;
}

foreign_ptr procarg0(self_ptr builder, foreign_ptr arg) { return nullptr; }

foreign_ptr prototype(self_ptr builder, foreign_ptr genargs, foreign_ptr args,
                      foreign_ptr return_type) {
  return nullptr;
}

foreign_ptr range_exclusive(self_ptr builder, foreign_ptr lhs,
                            const token *oper, foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr range_inclusive(self_ptr builder, foreign_ptr lhs,
                            const token *oper, foreign_ptr rhs) {
  return nullptr;
}

foreign_ptr rational(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr rational_complex(self_ptr builder, const token *tok) {
  return nullptr;
}

foreign_ptr regexp_compose(self_ptr builder, const token *begin,
                           const node_list *parts, const token *end,
                           foreign_ptr options) {
  return nullptr;
}

foreign_ptr regexp_options(self_ptr builder, const token *regopt) {
  return nullptr;
}

foreign_ptr rescue_body(self_ptr builder, const token *rescue,
                        foreign_ptr exc_list, const token *assoc,
                        foreign_ptr exc_var, const token *then,
                        foreign_ptr body) {
  return nullptr;
}

foreign_ptr restarg(self_ptr builder, const token *star, const token *name) {
  return nullptr;
}

foreign_ptr self_(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr shadowarg(self_ptr builder, const token *name) { return nullptr; }

foreign_ptr splat(self_ptr builder, const token *star, foreign_ptr arg) {
  return nullptr;
}

foreign_ptr string(self_ptr builder, const token *string_) { return nullptr; }

foreign_ptr string_compose(self_ptr builder, const token *begin,
                           const node_list *parts, const token *end) {
  return nullptr;
}

foreign_ptr string_internal(self_ptr builder, const token *string_) {
  return nullptr;
}

foreign_ptr symbol(self_ptr builder, const token *symbol) { return nullptr; }

foreign_ptr symbol_compose(self_ptr builder, const token *begin,
                           const node_list *parts, const token *end) {
  return nullptr;
}

foreign_ptr symbol_internal(self_ptr builder, const token *symbol) {
  return nullptr;
}

foreign_ptr symbols_compose(self_ptr builder, const token *begin,
                            const node_list *parts, const token *end) {
  return nullptr;
}

foreign_ptr ternary(self_ptr builder, foreign_ptr cond, const token *question,
                    foreign_ptr if_true, const token *colon,
                    foreign_ptr if_false) {
  return nullptr;
}

foreign_ptr tr_any(self_ptr builder, const token *special) { return nullptr; }

foreign_ptr tr_arg_instance(self_ptr builder, foreign_ptr base,
                            const node_list *types, const token *end) {
  return nullptr;
}

foreign_ptr tr_array(self_ptr builder, const token *begin, foreign_ptr type_,
                     const token *end) {
  return nullptr;
}

foreign_ptr tr_cast(self_ptr builder, const token *begin, foreign_ptr expr,
                    const token *colon, foreign_ptr type_, const token *end) {
  return nullptr;
}

foreign_ptr tr_class(self_ptr builder, const token *special) { return nullptr; }

foreign_ptr tr_consubtype(self_ptr builder, foreign_ptr sub,
                          foreign_ptr super_) {
  return nullptr;
}

foreign_ptr tr_conunify(self_ptr builder, foreign_ptr a, foreign_ptr b) {
  return nullptr;
}

foreign_ptr tr_cpath(self_ptr builder, foreign_ptr cpath) { return nullptr; }

foreign_ptr tr_genargs(self_ptr builder, const token *begin,
                       const node_list *genargs, const node_list *constraints,
                       const token *end) {
  return nullptr;
}

foreign_ptr tr_gendecl(self_ptr builder, foreign_ptr cpath, const token *begin,
                       const node_list *genargs, const node_list *constraints,
                       const token *end) {
  return nullptr;
}

foreign_ptr tr_gendeclarg(self_ptr builder, const token *tok,
                          foreign_ptr constraint) {
  return nullptr;
}

foreign_ptr tr_geninst(self_ptr builder, foreign_ptr cpath, const token *begin,
                       const node_list *genargs, const token *end) {
  return nullptr;
}

foreign_ptr tr_hash(self_ptr builder, const token *begin, foreign_ptr key_type,
                    const token *assoc, foreign_ptr value_type,
                    const token *end) {
  return nullptr;
}

foreign_ptr tr_instance(self_ptr builder, const token *special) {
  return nullptr;
}

foreign_ptr tr_ivardecl(self_ptr builder, const token *name,
                        foreign_ptr type_) {
  return nullptr;
}

foreign_ptr tr_nil(self_ptr builder, const token *nil) { return nullptr; }

foreign_ptr tr_nillable(self_ptr builder, const token *tilde,
                        foreign_ptr type_) {
  return nullptr;
}

foreign_ptr tr_or(self_ptr builder, foreign_ptr a, foreign_ptr b) {
  return nullptr;
}

foreign_ptr tr_proc(self_ptr builder, const token *begin, foreign_ptr args,
                    const token *end) {
  return nullptr;
}

foreign_ptr tr_self(self_ptr builder, const token *special) { return nullptr; }

foreign_ptr tr_tuple(self_ptr builder, const token *begin,
                     const node_list *types, const token *end) {
  return nullptr;
}

foreign_ptr true_(self_ptr builder, const token *tok) { return nullptr; }

foreign_ptr typed_arg(self_ptr builder, foreign_ptr type_, foreign_ptr arg) {
  return nullptr;
}

foreign_ptr unary_op(self_ptr builder, const token *oper,
                     foreign_ptr receiver) {
  return nullptr;
}

foreign_ptr undef_method(self_ptr builder, const token *undef,
                         const node_list *name_list) {
  return nullptr;
}

foreign_ptr when(self_ptr builder, const token *when, const node_list *patterns,
                 const token *then, foreign_ptr body) {
  return nullptr;
}

foreign_ptr word(self_ptr builder, const node_list *parts) { return nullptr; }

foreign_ptr words_compose(self_ptr builder, const token *begin,
                          const node_list *parts, const token *end) {
  return nullptr;
}

foreign_ptr xstring_compose(self_ptr builder, const token *begin,
                            const node_list *parts, const token *end) {
  return nullptr;
}

struct ruby_parser::builder builder = {
    accessible,
    alias,
    arg,
    args,
    array,
    assign,
    assignable,
    associate,
    attr_asgn,
    back_ref,
    begin,
    begin_body,
    begin_keyword,
    binary_op,
    block,
    block_pass,
    blockarg,
    call_lambda,
    call_method,
    case_,
    character,
    complex,
    compstmt,
    condition,
    condition_mod,
    const_,
    const_fetch,
    const_global,
    const_op_assignable,
    cvar,
    dedent_string,
    def_class,
    def_method,
    def_module,
    def_sclass,
    def_singleton,
    encoding_literal,
    false_,
    file_literal,
    float_,
    float_complex,
    for_,
    gvar,
    ident,
    index,
    index_asgn,
    integer,
    ivar,
    keyword_break,
    keyword_defined,
    keyword_next,
    keyword_redo,
    keyword_retry,
    keyword_return,
    keyword_super,
    keyword_yield,
    keyword_zsuper,
    kwarg,
    kwoptarg,
    kwrestarg,
    kwsplat,
    line_literal,
    logical_and,
    logical_or,
    loop_until,
    loop_until_mod,
    loop_while,
    loop_while_mod,
    match_op,
    multi_assign,
    multi_lhs,
    multi_lhs1,
    negate,
    nil,
    not_op,
    nth_ref,
    op_assign,
    optarg,
    pair,
    pair_keyword,
    pair_quoted,
    postexe,
    preexe,
    procarg0,
    prototype,
    range_exclusive,
    range_inclusive,
    rational,
    rational_complex,
    regexp_compose,
    regexp_options,
    rescue_body,
    restarg,
    self_,
    shadowarg,
    splat,
    string,
    string_compose,
    string_internal,
    symbol,
    symbol_compose,
    symbol_internal,
    symbols_compose,
    ternary,
    tr_any,
    tr_arg_instance,
    tr_array,
    tr_cast,
    tr_class,
    tr_consubtype,
    tr_conunify,
    tr_cpath,
    tr_genargs,
    tr_gendecl,
    tr_gendeclarg,
    tr_geninst,
    tr_hash,
    tr_instance,
    tr_ivardecl,
    tr_nil,
    tr_nillable,
    tr_or,
    tr_proc,
    tr_self,
    tr_tuple,
    true_,
    typed_arg,
    unary_op,
    undef_method,
    when,
    word,
    words_compose,
    xstring_compose,
};
};
};
