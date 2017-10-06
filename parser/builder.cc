#include "parser/builder.h"
#include "parser/Trees.h"

#include "ruby_parser/builder.hh"

using ruby_parser::foreign_ptr;
using ruby_parser::self_ptr;
using ruby_parser::token;
using ruby_parser::node_list;

using sruby::ast::ContextBase;
using std::unique_ptr;

namespace sruby {
namespace parser {

class Builder::Impl {
public:
    Impl(ContextBase &ctx, Result &r) : ctx_(ctx), result_(r) {}

    ContextBase &ctx_;
    Result &result_;

    Loc tok_loc(const token *tok) {
        return Loc{(u4)tok->start(), (u4)tok->end()};
    }

    Node *accessible(Node *node) {
        return nullptr;
    }

    Node *alias(const token *alias, Node *to, Node *from) {
        return nullptr;
    }

    Node *arg(const token *name) {
        return nullptr;
    }

    Node *args(const token *begin, const node_list *args, const token *end, bool check_args) {
        return nullptr;
    }

    Node *array(const token *begin, const node_list *elements, const token *end) {
        return nullptr;
    }

    Node *assign(Node *lhs, const token *eql, Node *rhs) {
        return nullptr;
    }

    Node *assignable(Node *node) {
        return nullptr;
    }

    Node *associate(const token *begin, const node_list *pairs, const token *end) {
        return nullptr;
    }

    Node *attr_asgn(Node *receiver, const token *dot, const token *selector) {
        return nullptr;
    }

    Node *back_ref(const token *tok) {
        return nullptr;
    }

    Node *begin(const token *begin, Node *body, const token *end) {
        return nullptr;
    }

    Node *begin_body(Node *body, const node_list *rescue_bodies, const token *else_tok, Node *else_,
                     const token *ensure_tok, Node *ensure) {
        return nullptr;
    }

    Node *begin_keyword(const token *begin, Node *body, const token *end) {
        return nullptr;
    }

    Node *binary_op(Node *receiver, const token *oper, Node *arg) {
        return nullptr;
    }

    Node *block(Node *method_call, const token *begin, Node *args, Node *body, const token *end) {
        return nullptr;
    }

    Node *block_pass(const token *amper, Node *arg) {
        return nullptr;
    }

    Node *blockarg(const token *amper, const token *name) {
        return nullptr;
    }

    Node *call_lambda(const token *lambda) {
        return nullptr;
    }

    Node *call_method(Node *receiver, const token *dot, const token *selector, const token *lparen,
                      const node_list *args, const token *rparen) {
        return nullptr;
    }

    Node *case_(const token *case_, Node *expr, const node_list *when_bodies, const token *else_tok, Node *else_body,
                const token *end) {
        return nullptr;
    }

    Node *character(const token *char_) {
        return nullptr;
    }

    Node *complex(const token *tok) {
        return nullptr;
    }

    Node *compstmt(const node_list *node) {
        return nullptr;
    }

    Node *condition(const token *cond_tok, Node *cond, const token *then, Node *if_true, const token *else_,
                    Node *if_false, const token *end) {
        return nullptr;
    }

    Node *condition_mod(Node *if_true, Node *if_false, Node *cond) {
        return nullptr;
    }

    Node *const_(const token *name) {
        return nullptr;
    }

    Node *const_fetch(Node *scope, const token *colon, const token *name) {
        return nullptr;
    }

    Node *const_global(const token *colon, const token *name) {
        return nullptr;
    }

    Node *const_op_assignable(Node *node) {
        return nullptr;
    }

    Node *cvar(const token *tok) {
        return nullptr;
    }

    Node *dedent_string(Node *node, size_t dedent_level) {
        return nullptr;
    }

    Node *def_class(const token *class_, Node *name, const token *lt_, Node *superclass, Node *body,
                    const token *end_) {
        return nullptr;
    }

    Node *def_method(const token *def, const token *name, Node *args, Node *body, const token *end) {
        return nullptr;
    }

    Node *def_module(const token *module, Node *name, Node *body, const token *end_) {
        return nullptr;
    }

    Node *def_sclass(const token *class_, const token *lshft_, Node *expr, Node *body, const token *end_) {
        return nullptr;
    }

    Node *def_singleton(const token *def, Node *definee, const token *dot, const token *name, Node *args, Node *body,
                        const token *end) {
        return nullptr;
    }

    Node *encoding_literal(const token *tok) {
        return nullptr;
    }

    Node *false_(const token *tok) {
        return nullptr;
    }

    Node *file_literal(const token *tok) {
        return nullptr;
    }

    Node *float_(const token *tok) {
        return nullptr;
    }

    Node *float_complex(const token *tok) {
        return nullptr;
    }

    Node *for_(const token *for_, Node *iterator, const token *in_, Node *iteratee, const token *do_, Node *body,
               const token *end) {
        return nullptr;
    }

    Node *gvar(const token *tok) {
        return nullptr;
    }

    Node *ident(const token *tok) {
        return new Ident(tok_loc(tok), ctx_.enterNameUTF8(tok->string()));
    }

    Node *index(Node *receiver, const token *lbrack, const node_list *indexes, const token *rbrack) {
        return nullptr;
    }

    Node *index_asgn(Node *receiver, const token *lbrack, const node_list *indexes, const token *rbrack) {
        return nullptr;
    }

    Node *integer(const token *tok) {
        return nullptr;
    }

    Node *ivar(const token *tok) {
        return nullptr;
    }

    Node *keyword_break(const token *keyword, const token *lparen, const node_list *args, const token *rparen) {
        return nullptr;
    }

    Node *keyword_defined(const token *keyword, Node *arg) {
        return nullptr;
    }

    Node *keyword_next(const token *keyword, const token *lparen, const node_list *args, const token *rparen) {
        return nullptr;
    }

    Node *keyword_redo(const token *keyword) {
        return nullptr;
    }

    Node *keyword_retry(const token *keyword) {
        return nullptr;
    }

    Node *keyword_return(const token *keyword, const token *lparen, const node_list *args, const token *rparen) {
        return nullptr;
    }

    Node *keyword_super(const token *keyword, const token *lparen, const node_list *args, const token *rparen) {
        return nullptr;
    }

    Node *keyword_yield(const token *keyword, const token *lparen, const node_list *args, const token *rparen) {
        return nullptr;
    }

    Node *keyword_zsuper(const token *keyword) {
        return nullptr;
    }

    Node *kwarg(const token *name) {
        return nullptr;
    }

    Node *kwoptarg(const token *name, Node *value) {
        return nullptr;
    }

    Node *kwrestarg(const token *dstar, const token *name) {
        return nullptr;
    }

    Node *kwsplat(const token *dstar, Node *arg) {
        return nullptr;
    }

    Node *line_literal(const token *tok) {
        return nullptr;
    }

    Node *logical_and(Node *lhs, const token *op, Node *rhs) {
        return nullptr;
    }

    Node *logical_or(Node *lhs, const token *op, Node *rhs) {
        return nullptr;
    }

    Node *loop_until(const token *keyword, Node *cond, const token *do_, Node *body, const token *end) {
        return nullptr;
    }

    Node *loop_until_mod(Node *body, Node *cond) {
        return nullptr;
    }

    Node *loop_while(const token *keyword, Node *cond, const token *do_, Node *body, const token *end) {
        return nullptr;
    }

    Node *loop_while_mod(Node *body, Node *cond) {
        return nullptr;
    }

    Node *match_op(Node *receiver, const token *oper, Node *arg) {
        return nullptr;
    }

    Node *multi_assign(Node *mlhs, Node *rhs) {
        return nullptr;
    }

    Node *multi_lhs(const token *begin, const node_list *items, const token *end) {
        return nullptr;
    }

    Node *multi_lhs1(const token *begin, Node *item, const token *end) {
        return nullptr;
    }

    Node *negate(const token *uminus, Node *numeric) {
        return nullptr;
    }

    Node *nil(const token *tok) {
        return nullptr;
    }

    Node *not_op(const token *not_, const token *begin, Node *receiver, const token *end) {
        return nullptr;
    }

    Node *nth_ref(const token *tok) {
        return nullptr;
    }

    Node *op_assign(Node *lhs, const token *op, Node *rhs) {
        return nullptr;
    }

    Node *optarg_(const token *name, const token *eql, Node *value) {
        return nullptr;
    }

    Node *pair(Node *key, const token *assoc, Node *value) {
        return nullptr;
    }

    Node *pair_keyword(const token *key, Node *value) {
        return nullptr;
    }

    Node *pair_quoted(const token *begin, const node_list *parts, const token *end, Node *value) {
        return nullptr;
    }

    Node *postexe(const token *begin, Node *node, const token *rbrace) {
        return nullptr;
    }

    Node *preexe(const token *begin, Node *node, const token *rbrace) {
        return nullptr;
    }

    Node *procarg0(Node *arg) {
        return nullptr;
    }

    Node *prototype(Node *genargs, Node *args, Node *return_type) {
        return nullptr;
    }

    Node *range_exclusive(Node *lhs, const token *oper, Node *rhs) {
        return nullptr;
    }

    Node *range_inclusive(Node *lhs, const token *oper, Node *rhs) {
        return nullptr;
    }

    Node *rational(const token *tok) {
        return nullptr;
    }

    Node *rational_complex(const token *tok) {
        return nullptr;
    }

    Node *regexp_compose(const token *begin, const node_list *parts, const token *end, Node *options) {
        return nullptr;
    }

    Node *regexp_options(const token *regopt) {
        return nullptr;
    }

    Node *rescue_body(const token *rescue, Node *exc_list, const token *assoc, Node *exc_var, const token *then,
                      Node *body) {
        return nullptr;
    }

    Node *restarg(const token *star, const token *name) {
        return nullptr;
    }

    Node *self_(const token *tok) {
        return nullptr;
    }

    Node *shadowarg(const token *name) {
        return nullptr;
    }

    Node *splat(const token *star, Node *arg) {
        return nullptr;
    }

    Node *string(const token *string_) {
        return nullptr;
    }

    Node *string_compose(const token *begin, const node_list *parts, const token *end) {
        return nullptr;
    }

    Node *string_internal(const token *string_) {
        return nullptr;
    }

    Node *symbol(const token *symbol) {
        return nullptr;
    }

    Node *symbol_compose(const token *begin, const node_list *parts, const token *end) {
        return nullptr;
    }

    Node *symbol_internal(const token *symbol) {
        return nullptr;
    }

    Node *symbols_compose(const token *begin, const node_list *parts, const token *end) {
        return nullptr;
    }

    Node *ternary(Node *cond, const token *question, Node *if_true, const token *colon, Node *if_false) {
        return nullptr;
    }

    Node *tr_any(const token *special) {
        return nullptr;
    }

    Node *tr_arg_instance(Node *base, const node_list *types, const token *end) {
        return nullptr;
    }

    Node *tr_array(const token *begin, Node *type_, const token *end) {
        return nullptr;
    }

    Node *tr_cast(const token *begin, Node *expr, const token *colon, Node *type_, const token *end) {
        return nullptr;
    }

    Node *tr_class(const token *special) {
        return nullptr;
    }

    Node *tr_consubtype(Node *sub, Node *super_) {
        return nullptr;
    }

    Node *tr_conunify(Node *a, Node *b) {
        return nullptr;
    }

    Node *tr_cpath(Node *cpath) {
        return nullptr;
    }

    Node *tr_genargs(const token *begin, const node_list *genargs, const node_list *constraints, const token *end) {
        return nullptr;
    }

    Node *tr_gendecl(Node *cpath, const token *begin, const node_list *genargs, const node_list *constraints,
                     const token *end) {
        return nullptr;
    }

    Node *tr_gendeclarg(const token *tok, Node *constraint) {
        return nullptr;
    }

    Node *tr_geninst(Node *cpath, const token *begin, const node_list *genargs, const token *end) {
        return nullptr;
    }

    Node *tr_hash(const token *begin, Node *key_type, const token *assoc, Node *value_type, const token *end) {
        return nullptr;
    }

    Node *tr_instance(const token *special) {
        return nullptr;
    }

    Node *tr_ivardecl(const token *name, Node *type_) {
        return nullptr;
    }

    Node *tr_nil(const token *nil) {
        return nullptr;
    }

    Node *tr_nillable(const token *tilde, Node *type_) {
        return nullptr;
    }

    Node *tr_or(Node *a, Node *b) {
        return nullptr;
    }

    Node *tr_proc(const token *begin, Node *args, const token *end) {
        return nullptr;
    }

    Node *tr_self(const token *special) {
        return nullptr;
    }

    Node *tr_tuple(const token *begin, const node_list *types, const token *end) {
        return nullptr;
    }

    Node *true_(const token *tok) {
        return nullptr;
    }

    Node *typed_arg(Node *type_, Node *arg) {
        return nullptr;
    }

    Node *unary_op(const token *oper, Node *receiver) {
        return nullptr;
    }

    Node *undef_method(const token *undef, const node_list *name_list) {
        return nullptr;
    }

    Node *when(const token *when, const node_list *patterns, const token *then, Node *body) {
        return nullptr;
    }

    Node *word(const node_list *parts) {
        return nullptr;
    }

    Node *words_compose(const token *begin, const node_list *parts, const token *end) {
        return nullptr;
    }

    Node *xstring_compose(const token *begin, const node_list *parts, const token *end) {
        return nullptr;
    }
};

Builder::Builder(ContextBase &ctx, Result &r) : impl_(new Builder::Impl(ctx, r)) {}
Builder::~Builder() {}

}; // namespace parser
}; // namespace sruby

namespace {

using sruby::parser::Builder;
using sruby::parser::Node;

Builder::Impl *cast_builder(self_ptr builder) {
    return const_cast<Builder::Impl *>(reinterpret_cast<const Builder::Impl *>(builder));
}

Node *cast_node(foreign_ptr node) {
    return const_cast<Node *>(reinterpret_cast<const Node *>(node));
}

foreign_ptr accessible(self_ptr builder, foreign_ptr node) {
    return cast_builder(builder)->accessible(cast_node(node));
}

foreign_ptr alias(self_ptr builder, const token *alias, foreign_ptr to, foreign_ptr from) {
    return nullptr;
}

foreign_ptr arg(self_ptr builder, const token *name) {
    return nullptr;
}

foreign_ptr args(self_ptr builder, const token *begin, const node_list *args, const token *end, bool check_args) {
    return nullptr;
}

foreign_ptr array(self_ptr builder, const token *begin, const node_list *elements, const token *end) {
    return nullptr;
}

foreign_ptr assign(self_ptr builder, foreign_ptr lhs, const token *eql, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr assignable(self_ptr builder, foreign_ptr node) {
    return nullptr;
}

foreign_ptr associate(self_ptr builder, const token *begin, const node_list *pairs, const token *end) {
    return nullptr;
}

foreign_ptr attr_asgn(self_ptr builder, foreign_ptr receiver, const token *dot, const token *selector) {
    return nullptr;
}

foreign_ptr back_ref(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr begin(self_ptr builder, const token *begin, foreign_ptr body, const token *end) {
    return nullptr;
}

foreign_ptr begin_body(self_ptr builder, foreign_ptr body, const node_list *rescue_bodies, const token *else_tok,
                       foreign_ptr else_, const token *ensure_tok, foreign_ptr ensure) {
    return nullptr;
}

foreign_ptr begin_keyword(self_ptr builder, const token *begin, foreign_ptr body, const token *end) {
    return nullptr;
}

foreign_ptr binary_op(self_ptr builder, foreign_ptr receiver, const token *oper, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr block(self_ptr builder, foreign_ptr method_call, const token *begin, foreign_ptr args, foreign_ptr body,
                  const token *end) {
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

foreign_ptr call_method(self_ptr builder, foreign_ptr receiver, const token *dot, const token *selector,
                        const token *lparen, const node_list *args, const token *rparen) {
    return nullptr;
}

foreign_ptr case_(self_ptr builder, const token *case_, foreign_ptr expr, const node_list *when_bodies,
                  const token *else_tok, foreign_ptr else_body, const token *end) {
    return nullptr;
}

foreign_ptr character(self_ptr builder, const token *char_) {
    return nullptr;
}

foreign_ptr complex(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr compstmt(self_ptr builder, const node_list *node) {
    return nullptr;
}

foreign_ptr condition(self_ptr builder, const token *cond_tok, foreign_ptr cond, const token *then, foreign_ptr if_true,
                      const token *else_, foreign_ptr if_false, const token *end) {
    return nullptr;
}

foreign_ptr condition_mod(self_ptr builder, foreign_ptr if_true, foreign_ptr if_false, foreign_ptr cond) {
    return nullptr;
}

foreign_ptr const_(self_ptr builder, const token *name) {
    return nullptr;
}

foreign_ptr const_fetch(self_ptr builder, foreign_ptr scope, const token *colon, const token *name) {
    return nullptr;
}

foreign_ptr const_global(self_ptr builder, const token *colon, const token *name) {
    return nullptr;
}

foreign_ptr const_op_assignable(self_ptr builder, foreign_ptr node) {
    return nullptr;
}

foreign_ptr cvar(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr dedent_string(self_ptr builder, foreign_ptr node, size_t dedent_level) {
    return nullptr;
}

foreign_ptr def_class(self_ptr builder, const token *class_, foreign_ptr name, const token *lt_, foreign_ptr superclass,
                      foreign_ptr body, const token *end_) {
    return nullptr;
}

foreign_ptr def_method(self_ptr builder, const token *def, const token *name, foreign_ptr args, foreign_ptr body,
                       const token *end) {
    return nullptr;
}

foreign_ptr def_module(self_ptr builder, const token *module, foreign_ptr name, foreign_ptr body, const token *end_) {
    return nullptr;
}

foreign_ptr def_sclass(self_ptr builder, const token *class_, const token *lshft_, foreign_ptr expr, foreign_ptr body,
                       const token *end_) {
    return nullptr;
}

foreign_ptr def_singleton(self_ptr builder, const token *def, foreign_ptr definee, const token *dot, const token *name,
                          foreign_ptr args, foreign_ptr body, const token *end) {
    return nullptr;
}

foreign_ptr encoding_literal(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr false_(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr file_literal(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr float_(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr float_complex(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr for_(self_ptr builder, const token *for_, foreign_ptr iterator, const token *in_, foreign_ptr iteratee,
                 const token *do_, foreign_ptr body, const token *end) {
    return nullptr;
}

foreign_ptr gvar(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr ident(self_ptr builder, const token *tok) {
    return cast_builder(builder)->ident(tok);
}

foreign_ptr index(self_ptr builder, foreign_ptr receiver, const token *lbrack, const node_list *indexes,
                  const token *rbrack) {
    return nullptr;
}

foreign_ptr index_asgn(self_ptr builder, foreign_ptr receiver, const token *lbrack, const node_list *indexes,
                       const token *rbrack) {
    return nullptr;
}

foreign_ptr integer(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr ivar(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr keyword_break(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    return nullptr;
}

foreign_ptr keyword_defined(self_ptr builder, const token *keyword, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr keyword_next(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                         const token *rparen) {
    return nullptr;
}

foreign_ptr keyword_redo(self_ptr builder, const token *keyword) {
    return nullptr;
}

foreign_ptr keyword_retry(self_ptr builder, const token *keyword) {
    return nullptr;
}

foreign_ptr keyword_return(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                           const token *rparen) {
    return nullptr;
}

foreign_ptr keyword_super(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    return nullptr;
}

foreign_ptr keyword_yield(self_ptr builder, const token *keyword, const token *lparen, const node_list *args,
                          const token *rparen) {
    return nullptr;
}

foreign_ptr keyword_zsuper(self_ptr builder, const token *keyword) {
    return nullptr;
}

foreign_ptr kwarg(self_ptr builder, const token *name) {
    return nullptr;
}

foreign_ptr kwoptarg(self_ptr builder, const token *name, foreign_ptr value) {
    return nullptr;
}

foreign_ptr kwrestarg(self_ptr builder, const token *dstar, const token *name) {
    return nullptr;
}

foreign_ptr kwsplat(self_ptr builder, const token *dstar, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr line_literal(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr logical_and(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr logical_or(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr loop_until(self_ptr builder, const token *keyword, foreign_ptr cond, const token *do_, foreign_ptr body,
                       const token *end) {
    return nullptr;
}

foreign_ptr loop_until_mod(self_ptr builder, foreign_ptr body, foreign_ptr cond) {
    return nullptr;
}

foreign_ptr loop_while(self_ptr builder, const token *keyword, foreign_ptr cond, const token *do_, foreign_ptr body,
                       const token *end) {
    return nullptr;
}

foreign_ptr loop_while_mod(self_ptr builder, foreign_ptr body, foreign_ptr cond) {
    return nullptr;
}

foreign_ptr match_op(self_ptr builder, foreign_ptr receiver, const token *oper, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr multi_assign(self_ptr builder, foreign_ptr mlhs, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr multi_lhs(self_ptr builder, const token *begin, const node_list *items, const token *end) {
    return nullptr;
}

foreign_ptr multi_lhs1(self_ptr builder, const token *begin, foreign_ptr item, const token *end) {
    return nullptr;
}

foreign_ptr negate(self_ptr builder, const token *uminus, foreign_ptr numeric) {
    return nullptr;
}

foreign_ptr nil(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr not_op(self_ptr builder, const token *not_, const token *begin, foreign_ptr receiver, const token *end) {
    return nullptr;
}

foreign_ptr nth_ref(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr op_assign(self_ptr builder, foreign_ptr lhs, const token *op, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr optarg_(self_ptr builder, const token *name, const token *eql, foreign_ptr value) {
    return nullptr;
}

foreign_ptr pair(self_ptr builder, foreign_ptr key, const token *assoc, foreign_ptr value) {
    return nullptr;
}

foreign_ptr pair_keyword(self_ptr builder, const token *key, foreign_ptr value) {
    return nullptr;
}

foreign_ptr pair_quoted(self_ptr builder, const token *begin, const node_list *parts, const token *end,
                        foreign_ptr value) {
    return nullptr;
}

foreign_ptr postexe(self_ptr builder, const token *begin, foreign_ptr node, const token *rbrace) {
    return nullptr;
}

foreign_ptr preexe(self_ptr builder, const token *begin, foreign_ptr node, const token *rbrace) {
    return nullptr;
}

foreign_ptr procarg0(self_ptr builder, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr prototype(self_ptr builder, foreign_ptr genargs, foreign_ptr args, foreign_ptr return_type) {
    return nullptr;
}

foreign_ptr range_exclusive(self_ptr builder, foreign_ptr lhs, const token *oper, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr range_inclusive(self_ptr builder, foreign_ptr lhs, const token *oper, foreign_ptr rhs) {
    return nullptr;
}

foreign_ptr rational(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr rational_complex(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr regexp_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end,
                           foreign_ptr options) {
    return nullptr;
}

foreign_ptr regexp_options(self_ptr builder, const token *regopt) {
    return nullptr;
}

foreign_ptr rescue_body(self_ptr builder, const token *rescue, foreign_ptr exc_list, const token *assoc,
                        foreign_ptr exc_var, const token *then, foreign_ptr body) {
    return nullptr;
}

foreign_ptr restarg(self_ptr builder, const token *star, const token *name) {
    return nullptr;
}

foreign_ptr self_(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr shadowarg(self_ptr builder, const token *name) {
    return nullptr;
}

foreign_ptr splat(self_ptr builder, const token *star, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr string(self_ptr builder, const token *string_) {
    return nullptr;
}

foreign_ptr string_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return nullptr;
}

foreign_ptr string_internal(self_ptr builder, const token *string_) {
    return nullptr;
}

foreign_ptr symbol(self_ptr builder, const token *symbol) {
    return nullptr;
}

foreign_ptr symbol_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return nullptr;
}

foreign_ptr symbol_internal(self_ptr builder, const token *symbol) {
    return nullptr;
}

foreign_ptr symbols_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return nullptr;
}

foreign_ptr ternary(self_ptr builder, foreign_ptr cond, const token *question, foreign_ptr if_true, const token *colon,
                    foreign_ptr if_false) {
    return nullptr;
}

foreign_ptr tr_any(self_ptr builder, const token *special) {
    return nullptr;
}

foreign_ptr tr_arg_instance(self_ptr builder, foreign_ptr base, const node_list *types, const token *end) {
    return nullptr;
}

foreign_ptr tr_array(self_ptr builder, const token *begin, foreign_ptr type_, const token *end) {
    return nullptr;
}

foreign_ptr tr_cast(self_ptr builder, const token *begin, foreign_ptr expr, const token *colon, foreign_ptr type_,
                    const token *end) {
    return nullptr;
}

foreign_ptr tr_class(self_ptr builder, const token *special) {
    return nullptr;
}

foreign_ptr tr_consubtype(self_ptr builder, foreign_ptr sub, foreign_ptr super_) {
    return nullptr;
}

foreign_ptr tr_conunify(self_ptr builder, foreign_ptr a, foreign_ptr b) {
    return nullptr;
}

foreign_ptr tr_cpath(self_ptr builder, foreign_ptr cpath) {
    return nullptr;
}

foreign_ptr tr_genargs(self_ptr builder, const token *begin, const node_list *genargs, const node_list *constraints,
                       const token *end) {
    return nullptr;
}

foreign_ptr tr_gendecl(self_ptr builder, foreign_ptr cpath, const token *begin, const node_list *genargs,
                       const node_list *constraints, const token *end) {
    return nullptr;
}

foreign_ptr tr_gendeclarg(self_ptr builder, const token *tok, foreign_ptr constraint) {
    return nullptr;
}

foreign_ptr tr_geninst(self_ptr builder, foreign_ptr cpath, const token *begin, const node_list *genargs,
                       const token *end) {
    return nullptr;
}

foreign_ptr tr_hash(self_ptr builder, const token *begin, foreign_ptr key_type, const token *assoc,
                    foreign_ptr value_type, const token *end) {
    return nullptr;
}

foreign_ptr tr_instance(self_ptr builder, const token *special) {
    return nullptr;
}

foreign_ptr tr_ivardecl(self_ptr builder, const token *name, foreign_ptr type_) {
    return nullptr;
}

foreign_ptr tr_nil(self_ptr builder, const token *nil) {
    return nullptr;
}

foreign_ptr tr_nillable(self_ptr builder, const token *tilde, foreign_ptr type_) {
    return nullptr;
}

foreign_ptr tr_or(self_ptr builder, foreign_ptr a, foreign_ptr b) {
    return nullptr;
}

foreign_ptr tr_proc(self_ptr builder, const token *begin, foreign_ptr args, const token *end) {
    return nullptr;
}

foreign_ptr tr_self(self_ptr builder, const token *special) {
    return nullptr;
}

foreign_ptr tr_tuple(self_ptr builder, const token *begin, const node_list *types, const token *end) {
    return nullptr;
}

foreign_ptr true_(self_ptr builder, const token *tok) {
    return nullptr;
}

foreign_ptr typed_arg(self_ptr builder, foreign_ptr type_, foreign_ptr arg) {
    return nullptr;
}

foreign_ptr unary_op(self_ptr builder, const token *oper, foreign_ptr receiver) {
    return nullptr;
}

foreign_ptr undef_method(self_ptr builder, const token *undef, const node_list *name_list) {
    return nullptr;
}

foreign_ptr when(self_ptr builder, const token *when, const node_list *patterns, const token *then, foreign_ptr body) {
    return nullptr;
}

foreign_ptr word(self_ptr builder, const node_list *parts) {
    return nullptr;
}

foreign_ptr words_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return nullptr;
}

foreign_ptr xstring_compose(self_ptr builder, const token *begin, const node_list *parts, const token *end) {
    return nullptr;
}
}; // namespace

namespace sruby {
namespace parser {

unique_ptr<Node> Builder::build(ruby_parser::base_driver *driver) {
    return unique_ptr<Node>(cast_node(driver->parse(impl_.get())));
}

struct ruby_parser::builder Builder::interface = {
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
    optarg_,
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
}
} // namespace sruby
