#ifndef RUBY_PARSER_BUILDER_HH
#define RUBY_PARSER_BUILDER_HH

#include <vector>
#include <memory>
#include <type_traits>

#include "node.hh"
#include "token.hh"
#include "driver.hh"

namespace ruby_parser {

struct builder {
	foreign_ptr(*accessible)(self_ptr builder, foreign_ptr node);
	foreign_ptr(*alias)(self_ptr builder, const token* alias, foreign_ptr to, foreign_ptr from);
	foreign_ptr(*arg)(self_ptr builder, const token* name);
	foreign_ptr(*args)(self_ptr builder, const token* begin, const node_list* args, const token* end, bool check_args);
	foreign_ptr(*array)(self_ptr builder, const token* begin, const node_list* elements, const token* end);
	foreign_ptr(*assign)(self_ptr builder, foreign_ptr lhs, const token* eql, foreign_ptr rhs);
	foreign_ptr(*assignable)(self_ptr builder, foreign_ptr node);
	foreign_ptr(*associate)(self_ptr builder, const token* begin, const node_list* pairs, const token* end);
	foreign_ptr(*attr_asgn)(self_ptr builder, foreign_ptr receiver, const token* dot, const token* selector);
	foreign_ptr(*back_ref)(self_ptr builder, const token* tok);
	foreign_ptr(*begin)(self_ptr builder, const token* begin, foreign_ptr body, const token* end);
	foreign_ptr(*begin_body)(self_ptr builder, foreign_ptr body, const node_list* rescue_bodies, const token* else_tok, foreign_ptr else_, const token* ensure_tok, foreign_ptr ensure);
	foreign_ptr(*begin_keyword)(self_ptr builder, const token* begin, foreign_ptr body, const token* end);
	foreign_ptr(*binary_op)(self_ptr builder, foreign_ptr receiver, const token* oper, foreign_ptr arg);
	foreign_ptr(*block)(self_ptr builder, foreign_ptr method_call, const token* begin, foreign_ptr args, foreign_ptr body, const token* end);
	foreign_ptr(*block_pass)(self_ptr builder, const token* amper, foreign_ptr arg);
	foreign_ptr(*blockarg)(self_ptr builder, const token* amper, const token* name);
	foreign_ptr(*call_lambda)(self_ptr builder, const token* lambda);
	foreign_ptr(*call_method)(self_ptr builder, foreign_ptr receiver, const token* dot, const token* selector, const token* lparen, const node_list* args, const token* rparen);
	foreign_ptr(*case_)(self_ptr builder, const token* case_, foreign_ptr expr, const node_list* when_bodies, const token* else_tok, foreign_ptr else_body, const token* end);
	foreign_ptr(*character)(self_ptr builder, const token* char_);
	foreign_ptr(*complex)(self_ptr builder, const token* tok);
	foreign_ptr(*compstmt)(self_ptr builder, const node_list* node);
	foreign_ptr(*condition)(self_ptr builder, const token* cond_tok, foreign_ptr cond, const token* then, foreign_ptr if_true, const token* else_, foreign_ptr if_false, const token* end);
	foreign_ptr(*condition_mod)(self_ptr builder, foreign_ptr if_true, foreign_ptr if_false, foreign_ptr cond);
	foreign_ptr(*const_)(self_ptr builder, const token* name);
	foreign_ptr(*const_fetch)(self_ptr builder, foreign_ptr scope, const token* colon, const token* name);
	foreign_ptr(*const_global)(self_ptr builder, const token* colon, const token* name);
	foreign_ptr(*const_op_assignable)(self_ptr builder, foreign_ptr node);
	foreign_ptr(*cvar)(self_ptr builder, const token* tok);
	foreign_ptr(*dedent_string)(self_ptr builder, foreign_ptr node, size_t dedentLevel);
	foreign_ptr(*def_class)(self_ptr builder, const token* class_, foreign_ptr name, const token* lt_, foreign_ptr superclass, foreign_ptr body, const token* end_);
	foreign_ptr(*def_method)(self_ptr builder, const token* def, const token* name, foreign_ptr args, foreign_ptr body, const token* end);
	foreign_ptr(*def_module)(self_ptr builder, const token* module, foreign_ptr name, foreign_ptr body, const token* end_);
	foreign_ptr(*def_sclass)(self_ptr builder, const token* class_, const token* lshft_, foreign_ptr expr, foreign_ptr body, const token* end_);
	foreign_ptr(*def_singleton)(self_ptr builder, const token* def, foreign_ptr definee, const token* dot, const token* name, foreign_ptr args, foreign_ptr body, const token* end);
	foreign_ptr(*encoding_literal)(self_ptr builder, const token* tok);
	foreign_ptr(*false_)(self_ptr builder, const token* tok);
	foreign_ptr(*file_literal)(self_ptr builder, const token* tok);
	foreign_ptr(*float_)(self_ptr builder, const token* tok);
	foreign_ptr(*float_complex)(self_ptr builder, const token* tok);
	foreign_ptr(*for_)(self_ptr builder, const token* for_, foreign_ptr iterator, const token* in_, foreign_ptr iteratee, const token* do_, foreign_ptr body, const token* end);
	foreign_ptr(*gvar)(self_ptr builder, const token* tok);
	foreign_ptr(*ident)(self_ptr builder, const token* tok);
	foreign_ptr(*index)(self_ptr builder, foreign_ptr receiver, const token* lbrack, const node_list* indexes, const token* rbrack);
	foreign_ptr(*index_asgn)(self_ptr builder, foreign_ptr receiver, const token* lbrack, const node_list* indexes, const token* rbrack);
	foreign_ptr(*integer)(self_ptr builder, const token* tok);
	foreign_ptr(*ivar)(self_ptr builder, const token* tok);
	foreign_ptr(*keyword_break)(self_ptr builder, const token* keyword, const token* lparen, const node_list* args, const token* rparen);
	foreign_ptr(*keyword_defined)(self_ptr builder, const token* keyword, foreign_ptr arg);
	foreign_ptr(*keyword_next)(self_ptr builder, const token* keyword, const token* lparen, const node_list* args, const token* rparen);
	foreign_ptr(*keyword_redo)(self_ptr builder, const token* keyword);
	foreign_ptr(*keyword_retry)(self_ptr builder, const token* keyword);
	foreign_ptr(*keyword_return)(self_ptr builder, const token* keyword, const token* lparen, const node_list* args, const token* rparen);
	foreign_ptr(*keyword_super)(self_ptr builder, const token* keyword, const token* lparen, const node_list* args, const token* rparen);
	foreign_ptr(*keyword_yield)(self_ptr builder, const token* keyword, const token* lparen, const node_list* args, const token* rparen);
	foreign_ptr(*keyword_zsuper)(self_ptr builder, const token* keyword);
	foreign_ptr(*kwarg)(self_ptr builder, const token* name);
	foreign_ptr(*kwoptarg)(self_ptr builder, const token* name, foreign_ptr value);
	foreign_ptr(*kwrestarg)(self_ptr builder, const token* dstar, const token* name);
	foreign_ptr(*kwsplat)(self_ptr builder, const token* dstar, foreign_ptr arg);
	foreign_ptr(*line_literal)(self_ptr builder, const token* tok);
	foreign_ptr(*logical_and)(self_ptr builder, foreign_ptr lhs, const token* op, foreign_ptr rhs);
	foreign_ptr(*logical_or)(self_ptr builder, foreign_ptr lhs, const token* op, foreign_ptr rhs);
	foreign_ptr(*loop_until)(self_ptr builder, const token* keyword, foreign_ptr cond, const token* do_, foreign_ptr body, const token* end);
	foreign_ptr(*loop_until_mod)(self_ptr builder, foreign_ptr body, foreign_ptr cond);
	foreign_ptr(*loop_while)(self_ptr builder, const token* keyword, foreign_ptr cond, const token* do_, foreign_ptr body, const token* end);
	foreign_ptr(*loop_while_mod)(self_ptr builder, foreign_ptr body, foreign_ptr cond);
	foreign_ptr(*match_op)(self_ptr builder, foreign_ptr receiver, const token* oper, foreign_ptr arg);
	foreign_ptr(*multi_assign)(self_ptr builder, foreign_ptr mlhs, foreign_ptr rhs);
	foreign_ptr(*multi_lhs)(self_ptr builder, const token* begin, const node_list* items, const token* end);
	foreign_ptr(*multi_lhs1)(self_ptr builder, const token* begin, foreign_ptr item, const token* end);
	foreign_ptr(*negate)(self_ptr builder, const token* uminus, foreign_ptr numeric);
	foreign_ptr(*nil)(self_ptr builder, const token* tok);
	foreign_ptr(*not_op)(self_ptr builder, const token* not_, const token* begin, foreign_ptr receiver, const token* end);
	foreign_ptr(*nth_ref)(self_ptr builder, const token* tok);
	foreign_ptr(*op_assign)(self_ptr builder, foreign_ptr lhs, const token* op, foreign_ptr rhs);
	foreign_ptr(*optarg)(self_ptr builder, const token* name, const token* eql, foreign_ptr value);
	foreign_ptr(*pair)(self_ptr builder, foreign_ptr key, const token* assoc, foreign_ptr value);
	foreign_ptr(*pair_keyword)(self_ptr builder, const token* key, foreign_ptr value);
	foreign_ptr(*pair_quoted)(self_ptr builder, const token* begin, const node_list* parts, const token* end, foreign_ptr value);
	foreign_ptr(*postexe)(self_ptr builder, const token* begin, foreign_ptr node, const token* rbrace);
	foreign_ptr(*preexe)(self_ptr builder, const token* begin, foreign_ptr node, const token* rbrace);
	foreign_ptr(*procarg0)(self_ptr builder, foreign_ptr arg);
	foreign_ptr(*range_exclusive)(self_ptr builder, foreign_ptr lhs, const token* oper, foreign_ptr rhs);
	foreign_ptr(*range_inclusive)(self_ptr builder, foreign_ptr lhs, const token* oper, foreign_ptr rhs);
	foreign_ptr(*rational)(self_ptr builder, const token* tok);
	foreign_ptr(*rational_complex)(self_ptr builder, const token* tok);
	foreign_ptr(*regexp_compose)(self_ptr builder, const token* begin, const node_list* parts, const token* end, foreign_ptr options);
	foreign_ptr(*regexp_options)(self_ptr builder, const token* regopt);
	foreign_ptr(*rescue_body)(self_ptr builder, const token* rescue, foreign_ptr exc_list, const token* assoc, foreign_ptr exc_var, const token* then, foreign_ptr body);
	foreign_ptr(*restarg)(self_ptr builder, const token* star, const token* name);
	foreign_ptr(*self_)(self_ptr builder, const token* tok);
	foreign_ptr(*shadowarg)(self_ptr builder, const token* name);
	foreign_ptr(*splat)(self_ptr builder, const token* star, foreign_ptr arg);
	foreign_ptr(*splat_mlhs)(self_ptr builder, const token* star, foreign_ptr arg);
	foreign_ptr(*string)(self_ptr builder, const token* string_);
	foreign_ptr(*string_compose)(self_ptr builder, const token* begin, const node_list* parts, const token* end);
	foreign_ptr(*string_internal)(self_ptr builder, const token* string_);
	foreign_ptr(*symbol)(self_ptr builder, const token* symbol);
	foreign_ptr(*symbol_compose)(self_ptr builder, const token* begin, const node_list* parts, const token* end);
	foreign_ptr(*symbol_internal)(self_ptr builder, const token* symbol);
	foreign_ptr(*symbols_compose)(self_ptr builder, const token* begin, const node_list* parts, const token* end);
	foreign_ptr(*ternary)(self_ptr builder, foreign_ptr cond, const token* question, foreign_ptr if_true, const token* colon, foreign_ptr if_false);
	foreign_ptr(*tr_any)(self_ptr builder, const token* special);
	foreign_ptr(*tr_arg_instance)(self_ptr builder, foreign_ptr base, const node_list* types, const token* end);
	foreign_ptr(*tr_array)(self_ptr builder, const token* begin, foreign_ptr type_, const token* end);
	foreign_ptr(*tr_cast)(self_ptr builder, const token* begin, foreign_ptr expr, const token* colon, foreign_ptr type_, const token* end);
	foreign_ptr(*tr_class)(self_ptr builder, const token* special);
	foreign_ptr(*tr_consubtype)(self_ptr builder, foreign_ptr sub, foreign_ptr super_);
	foreign_ptr(*tr_conunify)(self_ptr builder, foreign_ptr a, foreign_ptr b);
	foreign_ptr(*tr_cpath)(self_ptr builder, foreign_ptr cpath);
	foreign_ptr(*tr_genargs)(self_ptr builder, const token* begin, const node_list* genargs, const node_list* constraints, const token* end);
	foreign_ptr(*tr_gendecl)(self_ptr builder, foreign_ptr cpath, const token* begin, const node_list* genargs, const node_list* constraints, const token* end);
	foreign_ptr(*tr_gendeclarg)(self_ptr builder, const token* tok, foreign_ptr constraint);
	foreign_ptr(*tr_geninst)(self_ptr builder, foreign_ptr cpath, const token* begin, const node_list* genargs, const token* end);
	foreign_ptr(*tr_hash)(self_ptr builder, const token* begin, foreign_ptr key_type, const token* assoc, foreign_ptr value_type, const token* end);
	foreign_ptr(*tr_instance)(self_ptr builder, const token* special);
	foreign_ptr(*tr_ivardecl)(self_ptr builder, const token* def, const token* name, foreign_ptr type_);
	foreign_ptr(*tr_nil)(self_ptr builder, const token* nil);
	foreign_ptr(*tr_nillable)(self_ptr builder, const token* tilde, foreign_ptr type_);
	foreign_ptr(*tr_or)(self_ptr builder, foreign_ptr a, foreign_ptr b);
	foreign_ptr(*tr_paren)(self_ptr builder, const token* begin, foreign_ptr node, const token* end);
	foreign_ptr(*tr_proc)(self_ptr builder, const token* begin, foreign_ptr args, const token* end);
	foreign_ptr(*tr_prototype)(self_ptr builder, foreign_ptr genargs, foreign_ptr args, foreign_ptr return_type);
	foreign_ptr(*tr_returnsig)(self_ptr builder, const token* arrow, foreign_ptr ret);
	foreign_ptr(*tr_self)(self_ptr builder, const token* special);
	foreign_ptr(*tr_tuple)(self_ptr builder, const token* begin, const node_list* types, const token* end);
	foreign_ptr(*tr_typed_arg)(self_ptr builder, foreign_ptr type_, foreign_ptr arg);
	foreign_ptr(*true_)(self_ptr builder, const token* tok);
	foreign_ptr(*unary_op)(self_ptr builder, const token* oper, foreign_ptr receiver);
	foreign_ptr(*undef_method)(self_ptr builder, const token* undef, const node_list* name_list);
	foreign_ptr(*when)(self_ptr builder, const token* when, const node_list* patterns, const token* then, foreign_ptr body);
	foreign_ptr(*word)(self_ptr builder, const node_list* parts);
	foreign_ptr(*words_compose)(self_ptr builder, const token* begin, const node_list* parts, const token* end);
	foreign_ptr(*xstring_compose)(self_ptr builder, const token* begin, const node_list* parts, const token* end);
};

static_assert(std::is_pod<builder>::value, "`builder` must be a POD type");

}

#endif
