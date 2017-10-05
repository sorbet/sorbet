extern crate ruby_parser;
extern crate difference;

#[macro_use]
mod helpers;

use std::path::PathBuf;
use std::rc::Rc;
use difference::{Changeset, Difference};

const OPTIONS: ruby_parser::ParserOptions =
ruby_parser::ParserOptions {
  emit_file_vars_as_literals: true,
  emit_lambda: true,
  emit_procarg0: true,
  declare_env: &["foo", "bar", "baz"]
};


#[test]
fn parse_preexe() {
	let code = "BEGIN { 1 }";
	let sexp = r##"
(preexe
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_for_mlhs() {
	let code = "for a, b in foo; p a, b; end";
	let sexp = r##"
(for
  (mlhs
    (lvasgn :a)
    (lvasgn :b))
  (lvar :foo)
  (send nil :p
    (lvar :a)
    (lvar :b)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op() {
	let code = "foo + 1";
	let sexp = r##"
(send
  (lvar :foo) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_1() {
	let code = "foo - 1";
	let sexp = r##"
(send
  (lvar :foo) :-
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_2() {
	let code = "foo * 1";
	let sexp = r##"
(send
  (lvar :foo) :*
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_3() {
	let code = "foo / 1";
	let sexp = r##"
(send
  (lvar :foo) :/
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_4() {
	let code = "foo % 1";
	let sexp = r##"
(send
  (lvar :foo) :%
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_5() {
	let code = "foo ** 1";
	let sexp = r##"
(send
  (lvar :foo) :**
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_6() {
	let code = "foo | 1";
	let sexp = r##"
(send
  (lvar :foo) :|
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_7() {
	let code = "foo ^ 1";
	let sexp = r##"
(send
  (lvar :foo) :^
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_8() {
	let code = "foo & 1";
	let sexp = r##"
(send
  (lvar :foo) :&
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_9() {
	let code = "foo <=> 1";
	let sexp = r##"
(send
  (lvar :foo) :<=>
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_10() {
	let code = "foo < 1";
	let sexp = r##"
(send
  (lvar :foo) :<
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_11() {
	let code = "foo <= 1";
	let sexp = r##"
(send
  (lvar :foo) :<=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_12() {
	let code = "foo > 1";
	let sexp = r##"
(send
  (lvar :foo) :>
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_13() {
	let code = "foo >= 1";
	let sexp = r##"
(send
  (lvar :foo) :>=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_14() {
	let code = "foo == 1";
	let sexp = r##"
(send
  (lvar :foo) :==
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_15() {
	let code = "foo != 1";
	let sexp = r##"
(send
  (lvar :foo) :!=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_16() {
	let code = "foo === 1";
	let sexp = r##"
(send
  (lvar :foo) :===
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_17() {
	let code = "foo =~ 1";
	let sexp = r##"
(send
  (lvar :foo) :=~
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_18() {
	let code = "foo !~ 1";
	let sexp = r##"
(send
  (lvar :foo) :!~
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_19() {
	let code = "foo << 1";
	let sexp = r##"
(send
  (lvar :foo) :<<
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_binary_op_20() {
	let code = "foo >> 1";
	let sexp = r##"
(send
  (lvar :foo) :>>
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_arg_newline() {
	let code = "fun (1\n)";
	let sexp = r##"
(send nil :fun
  (begin
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_symbols() {
	let code = "%i[foo bar]";
	let sexp = r##"
(array
  (sym :foo)
  (sym :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_float() {
	let code = "1.33";
	let sexp = r##"
(float 1.33)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_float_1() {
	let code = "-1.33";
	let sexp = r##"
(float -1.33)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_restarg_unnamed() {
	let code = "def f(*); end";
	let sexp = r##"
(def :f
  (args
    (restarg)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ternary() {
	let code = "foo ? 1 : 2";
	let sexp = r##"
(if
  (lvar :foo)
  (int 1)
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_until_mod() {
	let code = "meth until foo";
	let sexp = r##"
(until
  (lvar :foo)
  (send nil :meth))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_empty_stmt() {
	let code = "";
	let sexp = r##"

"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_attr_asgn() {
	let code = "foo.a = 1";
	let sexp = r##"
(send
  (lvar :foo) :a=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_attr_asgn_1() {
	let code = "foo::a = 1";
	let sexp = r##"
(send
  (lvar :foo) :a=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_attr_asgn_2() {
	let code = "foo.A = 1";
	let sexp = r##"
(send
  (lvar :foo) :A=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_attr_asgn_3() {
	let code = "foo::A = 1";
	let sexp = r##"
(casgn
  (lvar :foo) :A
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_kwrestarg_named() {
	let code = "def f(**foo); end";
	let sexp = r##"
(def :f
  (args
    (kwrestarg :foo)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_call() {
	let code = "foo.(1)";
	let sexp = r##"
(send
  (lvar :foo) :call
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_call_1() {
	let code = "foo::(1)";
	let sexp = r##"
(send
  (lvar :foo) :call
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_case_expr_else() {
	let code = "case foo; when 'bar'; bar; else baz; end";
	let sexp = r##"
(case
  (lvar :foo)
  (when
    (str "bar")
    (lvar :bar))
  (lvar :baz))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_symbols_interp() {
	let code = "%I[foo #{bar}]";
	let sexp = r##"
(array
  (sym :foo)
  (dsym
    (begin
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_symbols_interp_1() {
	let code = "%I[foo#{bar}]";
	let sexp = r##"
(array
  (dsym
    (str "foo")
    (begin
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_lvasgn() {
	let code = "var = 10; var";
	let sexp = r##"
(begin
  (lvasgn :var
    (int 10))
  (lvar :var))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cvasgn() {
	let code = "@@var = 10";
	let sexp = r##"
(cvasgn :@@var
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_nil_expression() {
	let code = "()";
	let sexp = r##"
(begin)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_nil_expression_1() {
	let code = "begin end";
	let sexp = r##"
(kwbegin)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_unless_mod() {
	let code = "bar unless foo";
	let sexp = r##"
(if
  (lvar :foo) nil
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs() {
	let code = "f{ |foo: 1, bar: 2, **baz, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (kwoptarg :foo
      (int 1))
    (kwoptarg :bar
      (int 2))
    (kwrestarg :baz)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_1() {
	let code = "f{ |foo: 1, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (kwoptarg :foo
      (int 1))
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_2() {
	let code = "f{ |**baz, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (kwrestarg :baz)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_interp_single() {
	let code = "\"#{1}\"";
	let sexp = r##"
(dstr
  (begin
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_interp_single_1() {
	let code = "%W\"#{1}\"";
	let sexp = r##"
(array
  (dstr
    (begin
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_do_block_in_call_args() {
	let code = "bar def foo; self.each do end end";
	let sexp = r##"
(send nil :bar
  (def :foo
    (args)
    (block
      (send
        (self) :each)
      (args) nil)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ensure() {
	let code = "begin; meth; ensure; bar; end";
	let sexp = r##"
(kwbegin
  (ensure
    (send nil :meth)
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_pow_precedence() {
	let code = "-2 ** 10";
	let sexp = r##"
(send
  (send
    (int 2) :**
    (int 10)) :-@)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_pow_precedence_1() {
	let code = "-2.0 ** 10";
	let sexp = r##"
(send
  (send
    (float 2.0) :**
    (int 10)) :-@)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_ensure() {
	let code = "begin; meth; rescue; baz; ensure; bar; end";
	let sexp = r##"
(kwbegin
  (ensure
    (rescue
      (send nil :meth)
      (resbody nil nil
        (lvar :baz)) nil)
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_super() {
	let code = "super(foo)";
	let sexp = r##"
(super
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_super_1() {
	let code = "super foo";
	let sexp = r##"
(super
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_super_2() {
	let code = "super()";
	let sexp = r##"
(super)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_and_asgn() {
	let code = "foo.a &&= 1";
	let sexp = r##"
(and-asgn
  (send
    (lvar :foo) :a)
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_and_asgn_1() {
	let code = "foo[0, 1] &&= 2";
	let sexp = r##"
(and-asgn
  (send
    (lvar :foo) :[]
    (int 0)
    (int 1))
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cvar() {
	let code = "@@foo";
	let sexp = r##"
(cvar :@@foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_nil() {
	let code = "nil";
	let sexp = r##"
(nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12073() {
	let code = "a = 1; a b: 1";
	let sexp = r##"
(begin
  (lvasgn :a
    (int 1))
  (send nil :a
    (hash
      (pair
        (sym :b)
        (int 1)))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12073_1() {
	let code = "def foo raise; raise A::B, ''; end";
	let sexp = r##"
(def :foo
  (args
    (arg :raise))
  (send nil :raise
    (const
      (const nil :A) :B)
    (str "")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_index_cmd() {
	let code = "foo[m bar]";
	let sexp = r##"
(send
  (lvar :foo) :[]
  (send nil :m
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_true() {
	let code = "true";
	let sexp = r##"
(true)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_attr() {
	let code = "self.a, self[1, 2] = foo";
	let sexp = r##"
(masgn
  (mlhs
    (send
      (self) :a=)
    (send
      (self) :[]=
      (int 1)
      (int 2)))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_attr_1() {
	let code = "self::a, foo = foo";
	let sexp = r##"
(masgn
  (mlhs
    (send
      (self) :a=)
    (lvasgn :foo))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_attr_2() {
	let code = "self.A, foo = foo";
	let sexp = r##"
(masgn
  (mlhs
    (send
      (self) :A=)
    (lvasgn :foo))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn() {
	let code = "foo, bar = 1, 2";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :foo)
    (lvasgn :bar))
  (array
    (int 1)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_1() {
	let code = "(foo, bar) = 1, 2";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :foo)
    (lvasgn :bar))
  (array
    (int 1)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_2() {
	let code = "foo, bar, baz = 1, 2";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :foo)
    (lvasgn :bar)
    (lvasgn :baz))
  (array
    (int 1)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_splat() {
	let code = "[1, *foo, 2]";
	let sexp = r##"
(array
  (int 1)
  (splat
    (lvar :foo))
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_splat_1() {
	let code = "[1, *foo]";
	let sexp = r##"
(array
  (int 1)
  (splat
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_splat_2() {
	let code = "[*foo]";
	let sexp = r##"
(array
  (splat
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_10279() {
	let code = "{a: if true then 42 end}";
	let sexp = r##"
(hash
  (pair
    (sym :a)
    (if
      (true)
      (int 42) nil)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_encoding_() {
	let code = "__ENCODING__";
	let sexp = r##"
(const
  (const nil :Encoding) :UTF_8)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a() {
	let code = "a b{c d}, :e do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (sym :e))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_1() {
	let code = "a b{c(d)}, :e do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (sym :e))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_2() {
	let code = "a b(c d), :e do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (sym :e))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_3() {
	let code = "a b(c(d)), :e do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (sym :e))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_4() {
	let code = "a b{c d}, 1 do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (int 1))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_5() {
	let code = "a b{c(d)}, 1 do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (int 1))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_6() {
	let code = "a b(c d), 1 do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (int 1))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_7() {
	let code = "a b(c(d)), 1 do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (int 1))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_8() {
	let code = "a b{c d}, 1.0 do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (float 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_9() {
	let code = "a b{c(d)}, 1.0 do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (float 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_10() {
	let code = "a b(c d), 1.0 do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (float 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_11() {
	let code = "a b(c(d)), 1.0 do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (float 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_12() {
	let code = "a b{c d}, 1.0r do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (rational 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_13() {
	let code = "a b{c(d)}, 1.0r do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (rational 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_14() {
	let code = "a b(c d), 1.0r do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (rational 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_15() {
	let code = "a b(c(d)), 1.0r do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (rational 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_16() {
	let code = "a b{c d}, 1.0i do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (complex 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_17() {
	let code = "a b{c(d)}, 1.0i do end";
	let sexp = r##"
(block
  (send nil :a
    (block
      (send nil :b)
      (args)
      (send nil :c
        (send nil :d)))
    (complex 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_18() {
	let code = "a b(c d), 1.0i do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (complex 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_a_19() {
	let code = "a b(c(d)), 1.0i do end";
	let sexp = r##"
(block
  (send nil :a
    (send nil :b
      (send nil :c
        (send nil :d)))
    (complex 1.0))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_conditional() {
	let code = "a&.b";
	let sexp = r##"
(csend
  (send nil :a) :b)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_and_or_masgn() {
	let code = "foo && (a, b = bar)";
	let sexp = r##"
(and
  (lvar :foo)
  (begin
    (masgn
      (mlhs
        (lvasgn :a)
        (lvasgn :b))
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_and_or_masgn_1() {
	let code = "foo || (a, b = bar)";
	let sexp = r##"
(or
  (lvar :foo)
  (begin
    (masgn
      (mlhs
        (lvasgn :a)
        (lvasgn :b))
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_elsif() {
	let code = "if foo; bar; elsif baz; 1; else 2; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar)
  (if
    (lvar :baz)
    (int 1)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_masgn_24() {
	let code = "if (a, b = foo); end";
	let sexp = r##"
(if
  (begin
    (masgn
      (mlhs
        (lvasgn :a)
        (lvasgn :b))
      (lvar :foo))) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defined() {
	let code = "defined? foo";
	let sexp = r##"
(defined?
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defined_1() {
	let code = "defined?(foo)";
	let sexp = r##"
(defined?
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defined_2() {
	let code = "defined? @foo";
	let sexp = r##"
(defined?
  (ivar :@foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args() {
	let code = "def f (((a))); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (mlhs
        (arg :a)))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_1() {
	let code = "def f ((a, a1)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (arg :a)
      (arg :a1))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_2() {
	let code = "def f ((a, *r)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (arg :a)
      (restarg :r))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_3() {
	let code = "def f ((a, *r, p)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (arg :a)
      (restarg :r)
      (arg :p))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_4() {
	let code = "def f ((a, *)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (arg :a)
      (restarg))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_5() {
	let code = "def f ((a, *, p)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (arg :a)
      (restarg)
      (arg :p))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_6() {
	let code = "def f ((*r)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (restarg :r))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_7() {
	let code = "def f ((*r, p)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (restarg :r)
      (arg :p))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_8() {
	let code = "def f ((*)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (restarg))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_9() {
	let code = "def f ((*, p)); end";
	let sexp = r##"
(def :f
  (args
    (mlhs
      (restarg)
      (arg :p))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_op_asgn() {
	let code = "A += 1";
	let sexp = r##"
(op-asgn
  (casgn nil :A) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_op_asgn_1() {
	let code = "::A += 1";
	let sexp = r##"
(op-asgn
  (casgn
    (cbase) :A) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_op_asgn_2() {
	let code = "B::A += 1";
	let sexp = r##"
(op-asgn
  (casgn
    (const nil :B) :A) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_op_asgn_3() {
	let code = "def x; self::A ||= 1; end";
	let sexp = r##"
(def :x
  (args)
  (or-asgn
    (casgn
      (self) :A)
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_op_asgn_4() {
	let code = "def x; ::A ||= 1; end";
	let sexp = r##"
(def :x
  (args)
  (or-asgn
    (casgn
      (cbase) :A)
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_args_assocs_comma() {
	let code = "foo[bar, :baz => 1,]";
	let sexp = r##"
(send
  (lvar :foo) :[]
  (lvar :bar)
  (hash
    (pair
      (sym :baz)
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_kwsplat() {
	let code = "{ foo: 2, **bar }";
	let sexp = r##"
(hash
  (pair
    (sym :foo)
    (int 2))
  (kwsplat
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_args_comma() {
	let code = "foo[bar,]";
	let sexp = r##"
(send
  (lvar :foo) :[]
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_postexe() {
	let code = "END { 1 }";
	let sexp = r##"
(postexe
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_args() {
	let code = "->(a) { }";
	let sexp = r##"
(block
  (lambda)
  (args
    (arg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_args_1() {
	let code = "-> (a) { }";
	let sexp = r##"
(block
  (lambda)
  (args
    (arg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_string_plain() {
	let code = "'foobar'";
	let sexp = r##"
(str "foobar")
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_string_plain_1() {
	let code = "%q(foobar)";
	let sexp = r##"
(str "foobar")
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_casgn_unscoped() {
	let code = "Foo = 10";
	let sexp = r##"
(casgn nil :Foo
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cond_eflipflop() {
	let code = "if foo...bar; end";
	let sexp = r##"
(if
  (eflipflop
    (lvar :foo)
    (lvar :bar)) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_range_inclusive() {
	let code = "1..2";
	let sexp = r##"
(irange
  (int 1)
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_restarg_named() {
	let code = "def f(*foo); end";
	let sexp = r##"
(def :f
  (args
    (restarg :foo)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_arg_block() {
	let code = "fun (1) {}";
	let sexp = r##"
(block
  (send nil :fun
    (begin
      (int 1)))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_arg_block_1() {
	let code = "foo.fun (1) {}";
	let sexp = r##"
(block
  (send
    (lvar :foo) :fun
    (begin
      (int 1)))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_arg_block_2() {
	let code = "foo::fun (1) {}";
	let sexp = r##"
(block
  (send
    (lvar :foo) :fun
    (begin
      (int 1)))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_module() {
	let code = "module Foo; end";
	let sexp = r##"
(module
  (const nil :Foo) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_parser_bug_198() {
	let code = "[/()\\1/, ?#]";
	let sexp = r##"
(array
  (regexp
    (str "()\\1")
    (regopt))
  (str "#"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_back_ref() {
	let code = "$+";
	let sexp = r##"
(back-ref :$+)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_assocs() {
	let code = "fun(:foo => 1)";
	let sexp = r##"
(send nil :fun
  (hash
    (pair
      (sym :foo)
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_assocs_1() {
	let code = "fun(:foo => 1, &baz)";
	let sexp = r##"
(send nil :fun
  (hash
    (pair
      (sym :foo)
      (int 1)))
  (block-pass
    (lvar :baz)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_unless_else() {
	let code = "unless foo then bar; else baz; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :baz)
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_unless_else_1() {
	let code = "unless foo; bar; else baz; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :baz)
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_range_exclusive() {
	let code = "1...2";
	let sexp = r##"
(erange
  (int 1)
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_nested() {
	let code = "a, (b, c) = foo";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :a)
    (mlhs
      (lvasgn :b)
      (lvasgn :c)))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_nested_1() {
	let code = "((b, )) = foo";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :b))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self() {
	let code = "fun";
	let sexp = r##"
(send nil :fun)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self_1() {
	let code = "fun!";
	let sexp = r##"
(send nil :fun!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self_2() {
	let code = "fun(1)";
	let sexp = r##"
(send nil :fun
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_const() {
	let code = "self::A, foo = foo";
	let sexp = r##"
(masgn
  (mlhs
    (casgn
      (self) :A)
    (lvasgn :foo))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_const_1() {
	let code = "::A, foo = foo";
	let sexp = r##"
(masgn
  (mlhs
    (casgn
      (cbase) :A)
    (lvasgn :foo))
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_op_asgn() {
	let code = "a += 1";
	let sexp = r##"
(op-asgn
  (lvasgn :a) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_op_asgn_1() {
	let code = "@a |= 1";
	let sexp = r##"
(op-asgn
  (ivasgn :@a) :|
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_op_asgn_2() {
	let code = "@@var |= 10";
	let sexp = r##"
(op-asgn
  (cvasgn :@@var) :|
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_op_asgn_3() {
	let code = "def a; @@var |= 10; end";
	let sexp = r##"
(def :a
  (args)
  (op-asgn
    (cvasgn :@@var) :|
    (int 10)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11380() {
	let code = "p -> { :hello }, a: 1 do end";
	let sexp = r##"
(block
  (send nil :p
    (block
      (lambda)
      (args)
      (sym :hello))
    (hash
      (pair
        (sym :a)
        (int 1))))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_heredoc_do() {
	let code = "f <<-TABLE do\nTABLE\nend";
	let sexp = r##"
(block
  (send nil :f
    (dstr))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_while() {
	let code = "while foo do meth end";
	let sexp = r##"
(while
  (lvar :foo)
  (send nil :meth))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_while_1() {
	let code = "while foo; meth end";
	let sexp = r##"
(while
  (lvar :foo)
  (send nil :meth))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_args_assocs() {
	let code = "fun(foo, :foo => 1)";
	let sexp = r##"
(send nil :fun
  (lvar :foo)
  (hash
    (pair
      (sym :foo)
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_args_assocs_1() {
	let code = "fun(foo, :foo => 1, &baz)";
	let sexp = r##"
(send nil :fun
  (lvar :foo)
  (hash
    (pair
      (sym :foo)
      (int 1)))
  (block-pass
    (lvar :baz)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_else_useless() {
	let code = "begin; else; 2; end";
	let sexp = r##"
(kwbegin
  (begin
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_else_useless_1() {
	let code = "begin; 1; else; 2; end";
	let sexp = r##"
(kwbegin
  (int 1)
  (begin
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_else_useless_2() {
	let code = "begin; 1; 2; else; 3; end";
	let sexp = r##"
(kwbegin
  (int 1)
  (int 2)
  (begin
    (int 3)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_arg() {
	let code = "fun (1)";
	let sexp = r##"
(send nil :fun
  (begin
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_9669() {
	let code = "def a b:\nreturn\nend";
	let sexp = r##"
(def :a
  (args
    (kwarg :b))
  (return))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_9669_1() {
	let code = "o = {\na:\n1\n}";
	let sexp = r##"
(lvasgn :o
  (hash
    (pair
      (sym :a)
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cond_begin_masgn() {
	let code = "if (bar; a, b = foo); end";
	let sexp = r##"
(if
  (begin
    (lvar :bar)
    (masgn
      (mlhs
        (lvasgn :a)
        (lvasgn :b))
      (lvar :foo))) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_super_block() {
	let code = "super foo, bar do end";
	let sexp = r##"
(block
  (super
    (lvar :foo)
    (lvar :bar))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_super_block_1() {
	let code = "super do end";
	let sexp = r##"
(block
  (zsuper)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_scope() {
	let code = "def f(var = defined?(var)) var end";
	let sexp = r##"
(def :f
  (args
    (optarg :var
      (defined?
        (lvar :var))))
  (lvar :var))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_scope_1() {
	let code = "def f(var: defined?(var)) var end";
	let sexp = r##"
(def :f
  (args
    (kwoptarg :var
      (defined?
        (lvar :var))))
  (lvar :var))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_scope_2() {
	let code = "lambda{|;a|a}";
	let sexp = r##"
(block
  (send nil :lambda)
  (args
    (shadowarg :a))
  (lvar :a))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_rescue_empty_else() {
	let code = "begin; rescue LoadError; else; end";
	let sexp = r##"
(kwbegin
  (rescue nil
    (resbody
      (array
        (const nil :LoadError)) nil nil) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_args_star() {
	let code = "fun(foo, *bar)";
	let sexp = r##"
(send nil :fun
  (lvar :foo)
  (splat
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_args_star_1() {
	let code = "fun(foo, *bar, &baz)";
	let sexp = r##"
(send nil :fun
  (lvar :foo)
  (splat
    (lvar :bar))
  (block-pass
    (lvar :baz)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_index_asgn() {
	let code = "foo[1, 2] = 3";
	let sexp = r##"
(send
  (lvar :foo) :[]=
  (int 1)
  (int 2)
  (int 3))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_words_interp() {
	let code = "%W[foo #{bar}]";
	let sexp = r##"
(array
  (str "foo")
  (dstr
    (begin
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_words_interp_1() {
	let code = "%W[foo #{bar}foo#@baz]";
	let sexp = r##"
(array
  (str "foo")
  (dstr
    (begin
      (lvar :bar))
    (str "foo")
    (ivar :@baz)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12686() {
	let code = "f (g rescue nil)";
	let sexp = r##"
(send nil :f
  (begin
    (rescue
      (send nil :g)
      (resbody nil nil
        (nil)) nil)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_not_cmd() {
	let code = "not m foo";
	let sexp = r##"
(send
  (send nil :m
    (lvar :foo)) :!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_10() {
	let code = "def f foo:\n; end";
	let sexp = r##"
(def :f
  (args
    (kwarg :foo)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_11() {
	let code = "def f foo: -1\n; end";
	let sexp = r##"
(def :f
  (args
    (kwoptarg :foo
      (int -1))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_args_shadow() {
	let code = "->(a; foo, bar) { }";
	let sexp = r##"
(block
  (lambda)
  (args
    (arg :a)
    (shadowarg :foo)
    (shadowarg :bar)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_else() {
	let code = "if foo then bar; else baz; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar)
  (lvar :baz))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_else_1() {
	let code = "if foo; bar; else baz; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar)
  (lvar :baz))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_mod() {
	let code = "meth rescue bar";
	let sexp = r##"
(rescue
  (send nil :meth)
  (resbody nil nil
    (lvar :bar)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_regexp_encoding() {
	let code = "/\\xa8/n =~ \"\"";
	let sexp = r##"
(match-with-lvasgn
  (regexp
    (str "\\xa8")
    (regopt :n))
  (str ""))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_ascii_8bit_in_literal() {
	let code = "# coding:utf-8\n         \"\\xD0\\xBF\\xD1\\x80\\xD0\\xBE\\xD0\\xB2\\xD0\\xB5\\xD1\\x80\\xD0\\xBA\\xD0\\xB0\"";
	let sexp = r##"
(str "\u{43f}\u{440}\u{43e}\u{432}\u{435}\u{440}\u{43a}\u{430}")
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_resbody_list_var() {
	let code = "begin; meth; rescue foo => ex; bar; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody
      (array
        (lvar :foo))
      (lvasgn :ex)
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_else_ensure() {
	let code = "begin; meth; rescue; baz; else foo; ensure; bar end";
	let sexp = r##"
(kwbegin
  (ensure
    (rescue
      (send nil :meth)
      (resbody nil nil
        (lvar :baz))
      (lvar :foo))
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_self() {
	let code = "self";
	let sexp = r##"
(self)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue() {
	let code = "begin; meth; rescue; foo; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody nil nil
      (lvar :foo)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ternary_ambiguous_symbol() {
	let code = "t=1;(foo)?t:T";
	let sexp = r##"
(begin
  (lvasgn :t
    (int 1))
  (if
    (begin
      (lvar :foo))
    (lvar :t)
    (const nil :T)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_3() {
	let code = "f{ |a, b,| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (arg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_conditional() {
	let code = "foo&.bar {}";
	let sexp = r##"
(block
  (csend
    (lvar :foo) :bar)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_plain_cmd() {
	let code = "foo.fun bar";
	let sexp = r##"
(send
  (lvar :foo) :fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_plain_cmd_1() {
	let code = "foo::fun bar";
	let sexp = r##"
(send
  (lvar :foo) :fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_plain_cmd_2() {
	let code = "foo::Fun bar";
	let sexp = r##"
(send
  (lvar :foo) :Fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_assocs() {
	let code = "[ 1 => 2 ]";
	let sexp = r##"
(array
  (hash
    (pair
      (int 1)
      (int 2))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_assocs_1() {
	let code = "[ 1, 2 => 3 ]";
	let sexp = r##"
(array
  (int 1)
  (hash
    (pair
      (int 2)
      (int 3))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_4() {
	let code = "f{ |foo:| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (kwarg :foo)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_10653() {
	let code = "true ? 1.tap do |n| p n end : 0";
	let sexp = r##"
(if
  (true)
  (block
    (send
      (int 1) :tap)
    (args
      (procarg0 :n))
    (send nil :p
      (lvar :n)))
  (int 0))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_10653_1() {
	let code = "false ? raise {} : tap {}";
	let sexp = r##"
(if
  (false)
  (block
    (send nil :raise)
    (args) nil)
  (block
    (send nil :tap)
    (args) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_10653_2() {
	let code = "false ? raise do end : tap do end";
	let sexp = r##"
(if
  (false)
  (block
    (send nil :raise)
    (args) nil)
  (block
    (send nil :tap)
    (args) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_kwrestarg_unnamed() {
	let code = "def f(**); end";
	let sexp = r##"
(def :f
  (args
    (kwrestarg)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_retry() {
	let code = "retry";
	let sexp = r##"
(retry)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_redo() {
	let code = "redo";
	let sexp = r##"
(redo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defs() {
	let code = "def self.foo; end";
	let sexp = r##"
(defs
  (self) :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defs_1() {
	let code = "def self::foo; end";
	let sexp = r##"
(defs
  (self) :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defs_2() {
	let code = "def (foo).foo; end";
	let sexp = r##"
(defs
  (lvar :foo) :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defs_3() {
	let code = "def String.foo; end";
	let sexp = r##"
(defs
  (const nil :String) :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_defs_4() {
	let code = "def String::foo; end";
	let sexp = r##"
(defs
  (const nil :String) :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_gvar() {
	let code = "$foo";
	let sexp = r##"
(gvar :$foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_cmd_string_lookahead() {
	let code = "desc \"foo\" do end";
	let sexp = r##"
(block
  (send nil :desc
    (str "foo"))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_label_end() {
	let code = "{ 'foo': 2 }";
	let sexp = r##"
(hash
  (pair
    (sym :foo)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_label_end_1() {
	let code = "{ 'foo': 2, 'bar': {}}";
	let sexp = r##"
(hash
  (pair
    (sym :foo)
    (int 2))
  (pair
    (sym :bar)
    (hash)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_label_end_2() {
	let code = "f(a ? \"a\":1)";
	let sexp = r##"
(send nil :f
  (if
    (send nil :a)
    (str "a")
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_args_noparen() {
	let code = "-> a: 1 { }";
	let sexp = r##"
(block
  (lambda)
  (args
    (kwoptarg :a
      (int 1))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_args_noparen_1() {
	let code = "-> a: { }";
	let sexp = r##"
(block
  (lambda)
  (args
    (kwarg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_break() {
	let code = "break(foo)";
	let sexp = r##"
(break
  (begin
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_break_1() {
	let code = "break foo";
	let sexp = r##"
(break
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_break_2() {
	let code = "break()";
	let sexp = r##"
(break
  (begin))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_break_3() {
	let code = "break";
	let sexp = r##"
(break)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat() {
	let code = "@foo, @@bar = *foo";
	let sexp = r##"
(masgn
  (mlhs
    (ivasgn :@foo)
    (cvasgn :@@bar))
  (array
    (splat
      (lvar :foo))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_1() {
	let code = "a, b = *foo, bar";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :a)
    (lvasgn :b))
  (array
    (splat
      (lvar :foo))
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_2() {
	let code = "a, *b = bar";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :a)
    (splat
      (lvasgn :b)))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_3() {
	let code = "a, *b, c = bar";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :a)
    (splat
      (lvasgn :b))
    (lvasgn :c))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_4() {
	let code = "a, * = bar";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :a)
    (splat))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_5() {
	let code = "a, *, c = bar";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :a)
    (splat)
    (lvasgn :c))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_6() {
	let code = "*b = bar";
	let sexp = r##"
(masgn
  (mlhs
    (splat
      (lvasgn :b)))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_7() {
	let code = "*b, c = bar";
	let sexp = r##"
(masgn
  (mlhs
    (splat
      (lvasgn :b))
    (lvasgn :c))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_8() {
	let code = "* = bar";
	let sexp = r##"
(masgn
  (mlhs
    (splat))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_splat_9() {
	let code = "*, c, d = bar";
	let sexp = r##"
(masgn
  (mlhs
    (splat)
    (lvasgn :c)
    (lvasgn :d))
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_kwbegin_compstmt() {
	let code = "begin foo!; bar! end";
	let sexp = r##"
(kwbegin
  (send nil :foo!)
  (send nil :bar!))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_non_lvar_injecting_match() {
	let code = "/#{1}(?<match>bar)/ =~ 'bar'";
	let sexp = r##"
(send
  (regexp
    (begin
      (int 1))
    (str "(?<match>bar)")
    (regopt)) :=~
  (str "bar"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_plain() {
	let code = "[1, 2]";
	let sexp = r##"
(array
  (int 1)
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_or_asgn() {
	let code = "foo.a ||= 1";
	let sexp = r##"
(or-asgn
  (send
    (lvar :foo) :a)
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_or_asgn_1() {
	let code = "foo[0, 1] ||= 2";
	let sexp = r##"
(or-asgn
  (send
    (lvar :foo) :[]
    (int 0)
    (int 1))
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_parser_bug_272() {
	let code = "a @b do |c|;end";
	let sexp = r##"
(block
  (send nil :a
    (ivar :@b))
  (args
    (procarg0 :c)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_undef() {
	let code = "undef foo, :bar, :\"foo#{1}\"";
	let sexp = r##"
(undef
  (sym :foo)
  (sym :bar)
  (dsym
    (str "foo")
    (begin
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_or_asgn() {
	let code = "a ||= 1";
	let sexp = r##"
(or-asgn
  (lvasgn :a)
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_lambda_leakage() {
	let code = "->(scope) {}; scope";
	let sexp = r##"
(begin
  (block
    (lambda)
    (args
      (arg :scope)) nil)
  (send nil :scope))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cond_match_current_line() {
	let code = "if /wat/; end";
	let sexp = r##"
(if
  (match-current-line
    (regexp
      (str "wat")
      (regopt))) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_hashrocket() {
	let code = "{ 1 => 2 }";
	let sexp = r##"
(hash
  (pair
    (int 1)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_hashrocket_1() {
	let code = "{ 1 => 2, :foo => \"bar\" }";
	let sexp = r##"
(hash
  (pair
    (int 1)
    (int 2))
  (pair
    (sym :foo)
    (str "bar")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rational() {
	let code = "42r";
	let sexp = r##"
(rational 42)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rational_1() {
	let code = "42.1r";
	let sexp = r##"
(rational 42.1)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_case_cond() {
	let code = "case; when foo; 'foo'; end";
	let sexp = r##"
(case nil
  (when
    (lvar :foo)
    (str "foo")) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_symbols_empty() {
	let code = "%i[]";
	let sexp = r##"
(array)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_symbols_empty_1() {
	let code = "%I()";
	let sexp = r##"
(array)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_case_expr() {
	let code = "case foo; when 'bar'; bar; end";
	let sexp = r##"
(case
  (lvar :foo)
  (when
    (str "bar")
    (lvar :bar)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_for() {
	let code = "for a in foo do p a; end";
	let sexp = r##"
(for
  (lvasgn :a)
  (lvar :foo)
  (send nil :p
    (lvar :a)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_for_1() {
	let code = "for a in foo; p a; end";
	let sexp = r##"
(for
  (lvasgn :a)
  (lvar :foo)
  (send nil :p
    (lvar :a)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_resbody_var() {
	let code = "begin; meth; rescue => ex; bar; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody nil
      (lvasgn :ex)
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_resbody_var_1() {
	let code = "begin; meth; rescue => @ex; bar; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody nil
      (ivasgn :@ex)
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_casgn_toplevel() {
	let code = "::Foo = 10";
	let sexp = r##"
(casgn
  (cbase) :Foo
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_gvasgn() {
	let code = "$var = 10";
	let sexp = r##"
(gvasgn :$var
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_label() {
	let code = "{ foo: 2 }";
	let sexp = r##"
(hash
  (pair
    (sym :foo)
    (int 2)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_duplicate_ignored() {
	let code = "def foo(_, _); end";
	let sexp = r##"
(def :foo
  (args
    (arg :_)
    (arg :_)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_duplicate_ignored_1() {
	let code = "def foo(_a, _a); end";
	let sexp = r##"
(def :foo
  (args
    (arg :_a)
    (arg :_a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_legacy() {
  let opts = ruby_parser::ParserOptions {
    emit_file_vars_as_literals: true,
    emit_lambda: false,
    emit_procarg0: true,
    declare_env: &["foo", "bar", "baz"]
  };
  let code = "->{ }";
	let sexp = r##"
(block
  (send nil :lambda)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, opts);
}

#[test]
fn parse_begin_cmdarg() {
	let code = "p begin 1.times do 1 end end";
	let sexp = r##"
(send nil :p
  (kwbegin
    (block
      (send
        (int 1) :times)
      (args)
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_attr_asgn_conditional() {
	let code = "a&.b = 1";
	let sexp = r##"
(csend
  (send nil :a) :b=
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11873_b() {
	let code = "p p{p(p);p p}, tap do end";
	let sexp = r##"
(block
  (send nil :p
    (block
      (send nil :p)
      (args)
      (begin
        (send nil :p
          (send nil :p))
        (send nil :p
          (send nil :p))))
    (send nil :tap))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_and_asgn() {
	let code = "a &&= 1";
	let sexp = r##"
(and-asgn
  (lvasgn :a)
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_plain() {
	let code = "foo.fun";
	let sexp = r##"
(send
  (lvar :foo) :fun)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_plain_1() {
	let code = "foo::fun";
	let sexp = r##"
(send
  (lvar :foo) :fun)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_plain_2() {
	let code = "foo::Fun()";
	let sexp = r##"
(send
  (lvar :foo) :Fun)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_return() {
	let code = "return(foo)";
	let sexp = r##"
(return
  (begin
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_return_1() {
	let code = "return foo";
	let sexp = r##"
(return
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_return_2() {
	let code = "return()";
	let sexp = r##"
(return
  (begin))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_return_3() {
	let code = "return";
	let sexp = r##"
(return)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_xstring_interp() {
	let code = "`foo#{bar}baz`";
	let sexp = r##"
(xstr
  (str "foo")
  (begin
    (lvar :bar))
  (str "baz"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_unary_op() {
	let code = "-foo";
	let sexp = r##"
(send
  (lvar :foo) :-@)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_unary_op_1() {
	let code = "+foo";
	let sexp = r##"
(send
  (lvar :foo) :+@)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_unary_op_2() {
	let code = "~foo";
	let sexp = r##"
(send
  (lvar :foo) :~)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cpath() {
	let code = "module ::Foo; end";
	let sexp = r##"
(module
  (const
    (cbase) :Foo) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cpath_1() {
	let code = "module Bar::Foo; end";
	let sexp = r##"
(module
  (const
    (const nil :Bar) :Foo) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_int() {
	let code = "42";
	let sexp = r##"
(int 42)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_int_1() {
	let code = "-42";
	let sexp = r##"
(int -42)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_words() {
	let code = "%w[foo bar]";
	let sexp = r##"
(array
  (str "foo")
  (str "bar"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_xstring_plain() {
	let code = "`foobar`";
	let sexp = r##"
(xstr
  (str "foobar"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_index() {
	let code = "foo[1, 2]";
	let sexp = r##"
(send
  (lvar :foo) :[]
  (int 1)
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_symbol_interp() {
	let code = ":\"foo#{bar}baz\"";
	let sexp = r##"
(dsym
  (str "foo")
  (begin
    (lvar :bar))
  (str "baz"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bang() {
	let code = "!foo";
	let sexp = r##"
(send
  (lvar :foo) :!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_hash_empty() {
	let code = "{ }";
	let sexp = r##"
(hash)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cond_begin() {
	let code = "if (bar); foo; end";
	let sexp = r##"
(if
  (begin
    (lvar :bar))
  (lvar :foo) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg() {
	let code = "def f(foo); end";
	let sexp = r##"
(def :f
  (args
    (arg :foo)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_1() {
	let code = "def f(foo, bar); end";
	let sexp = r##"
(def :f
  (args
    (arg :foo)
    (arg :bar)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_12() {
	let code = "def f a, o=1, *r, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (restarg :r)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_13() {
	let code = "def f a, o=1, *r, p, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_14() {
	let code = "def f a, o=1, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_15() {
	let code = "def f a, o=1, p, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_16() {
	let code = "def f a, *r, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (restarg :r)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_17() {
	let code = "def f a, *r, p, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_18() {
	let code = "def f a, &b; end";
	let sexp = r##"
(def :f
  (args
    (arg :a)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_19() {
	let code = "def f o=1, *r, &b; end";
	let sexp = r##"
(def :f
  (args
    (optarg :o
      (int 1))
    (restarg :r)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_20() {
	let code = "def f o=1, *r, p, &b; end";
	let sexp = r##"
(def :f
  (args
    (optarg :o
      (int 1))
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_21() {
	let code = "def f o=1, &b; end";
	let sexp = r##"
(def :f
  (args
    (optarg :o
      (int 1))
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_22() {
	let code = "def f o=1, p, &b; end";
	let sexp = r##"
(def :f
  (args
    (optarg :o
      (int 1))
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_23() {
	let code = "def f *r, &b; end";
	let sexp = r##"
(def :f
  (args
    (restarg :r)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_24() {
	let code = "def f *r, p, &b; end";
	let sexp = r##"
(def :f
  (args
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_25() {
	let code = "def f &b; end";
	let sexp = r##"
(def :f
  (args
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_26() {
	let code = "def f ; end";
	let sexp = r##"
(def :f
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_zsuper() {
	let code = "super";
	let sexp = r##"
(zsuper)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_when_multi() {
	let code = "case foo; when 'bar', 'baz'; bar; end";
	let sexp = r##"
(case
  (lvar :foo)
  (when
    (str "bar")
    (str "baz")
    (lvar :bar)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_asgn_mrhs() {
	let code = "foo = bar, 1";
	let sexp = r##"
(lvasgn :foo
  (array
    (lvar :bar)
    (int 1)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_asgn_mrhs_1() {
	let code = "foo = *bar";
	let sexp = r##"
(lvasgn :foo
  (array
    (splat
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_asgn_mrhs_2() {
	let code = "foo = baz, *bar";
	let sexp = r##"
(lvasgn :foo
  (array
    (lvar :baz)
    (splat
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_kwarg() {
	let code = "def f(foo:); end";
	let sexp = r##"
(def :f
  (args
    (kwarg :foo)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_next() {
	let code = "next(foo)";
	let sexp = r##"
(next
  (begin
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_next_1() {
	let code = "next foo";
	let sexp = r##"
(next
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_next_2() {
	let code = "next()";
	let sexp = r##"
(next
  (begin))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_next_3() {
	let code = "next";
	let sexp = r##"
(next)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_27() {
	let code = "def f (foo: 1, bar: 2, **baz, &b); end";
	let sexp = r##"
(def :f
  (args
    (kwoptarg :foo
      (int 1))
    (kwoptarg :bar
      (int 2))
    (kwrestarg :baz)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_28() {
	let code = "def f (foo: 1, &b); end";
	let sexp = r##"
(def :f
  (args
    (kwoptarg :foo
      (int 1))
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_29() {
	let code = "def f **baz, &b; end";
	let sexp = r##"
(def :f
  (args
    (kwrestarg :baz)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_args_30() {
	let code = "def f *, **; end";
	let sexp = r##"
(def :f
  (args
    (restarg)
    (kwrestarg)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_mod_op_assign() {
	let code = "foo += meth rescue bar";
	let sexp = r##"
(op-asgn
  (lvasgn :foo) :+
  (rescue
    (send nil :meth)
    (resbody nil nil
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_return_block() {
	let code = "return fun foo do end";
	let sexp = r##"
(return
  (block
    (send nil :fun
      (lvar :foo))
    (args) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_break_block() {
	let code = "break fun foo do end";
	let sexp = r##"
(break
  (block
    (send nil :fun
      (lvar :foo))
    (args) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_asgn_cmd() {
	let code = "foo = m foo";
	let sexp = r##"
(lvasgn :foo
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_asgn_cmd_1() {
	let code = "foo = bar = m foo";
	let sexp = r##"
(lvasgn :foo
  (lvasgn :bar
    (send nil :m
      (lvar :foo))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_star() {
	let code = "fun(*bar)";
	let sexp = r##"
(send nil :fun
  (splat
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_star_1() {
	let code = "fun(*bar, &baz)";
	let sexp = r##"
(send nil :fun
  (splat
    (lvar :bar))
  (block-pass
    (lvar :baz)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_until_post() {
	let code = "begin meth end until foo";
	let sexp = r##"
(until-post
  (lvar :foo)
  (kwbegin
    (send nil :meth)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_5() {
  let opts =  ruby_parser::ParserOptions {
    emit_file_vars_as_literals: true,
    emit_lambda: true,
    emit_procarg0: false,
    declare_env: &["foo", "bar", "baz"]
  };
	let code = "f{ |a| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, opts);
}

#[test]
fn parse_when_splat() {
	let code = "case foo; when 1, *baz; bar; when *foo; end";
	let sexp = r##"
(case
  (lvar :foo)
  (when
    (int 1)
    (splat
      (lvar :baz))
    (lvar :bar))
  (when
    (splat
      (lvar :foo)) nil) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_index_cmd() {
	let code = "foo[0, 1] += m foo";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :[]
    (int 0)
    (int 1)) :+
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ivar() {
	let code = "@foo";
	let sexp = r##"
(ivar :@foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_arg_call() {
	let code = "fun (1).to_i";
	let sexp = r##"
(send nil :fun
  (send
    (begin
      (int 1)) :to_i))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_def_no_paren_eql_begin() {
	let code = "def foo\n=begin\n=end\nend";
	let sexp = r##"
(def :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_cmd() {
	let code = "fun(f bar)";
	let sexp = r##"
(send nil :fun
  (send nil :f
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_string_file_() {
	let code = "__FILE__";
	let sexp = r##"
(str "(assert_parses)")
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_blockarg() {
	let code = "def f(&block); end";
	let sexp = r##"
(def :f
  (args
    (blockarg :block)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_toplevel() {
	let code = "::Foo";
	let sexp = r##"
(const
  (cbase) :Foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd() {
	let code = "meth 1 do end.fun bar";
	let sexp = r##"
(send
  (block
    (send nil :meth
      (int 1))
    (args) nil) :fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd_1() {
	let code = "meth 1 do end.fun(bar)";
	let sexp = r##"
(send
  (block
    (send nil :meth
      (int 1))
    (args) nil) :fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd_2() {
	let code = "meth 1 do end::fun bar";
	let sexp = r##"
(send
  (block
    (send nil :meth
      (int 1))
    (args) nil) :fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd_3() {
	let code = "meth 1 do end::fun(bar)";
	let sexp = r##"
(send
  (block
    (send nil :meth
      (int 1))
    (args) nil) :fun
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd_4() {
	let code = "meth 1 do end.fun bar do end";
	let sexp = r##"
(block
  (send
    (block
      (send nil :meth
        (int 1))
      (args) nil) :fun
    (lvar :bar))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd_5() {
	let code = "meth 1 do end.fun(bar) {}";
	let sexp = r##"
(block
  (send
    (block
      (send nil :meth
        (int 1))
      (args) nil) :fun
    (lvar :bar))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_block_chain_cmd_6() {
	let code = "meth 1 do end.fun {}";
	let sexp = r##"
(block
  (send
    (block
      (send nil :meth
        (int 1))
      (args) nil) :fun)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11990() {
	let code = "p <<~E \"  y\"\n  x\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "x\n")
    (str "  y")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402() {
	let code = "foo = raise(bar) rescue nil";
	let sexp = r##"
(lvasgn :foo
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_1() {
	let code = "foo += raise(bar) rescue nil";
	let sexp = r##"
(op-asgn
  (lvasgn :foo) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_2() {
	let code = "foo[0] += raise(bar) rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :[]
    (int 0)) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_3() {
	let code = "foo.m += raise(bar) rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :m) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_4() {
	let code = "foo::m += raise(bar) rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :m) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_5() {
	let code = "foo.C += raise(bar) rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :C) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_6() {
	let code = "foo::C ||= raise(bar) rescue nil";
	let sexp = r##"
(or-asgn
  (casgn
    (lvar :foo) :C)
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_7() {
	let code = "foo = raise bar rescue nil";
	let sexp = r##"
(lvasgn :foo
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_8() {
	let code = "foo += raise bar rescue nil";
	let sexp = r##"
(op-asgn
  (lvasgn :foo) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_9() {
	let code = "foo[0] += raise bar rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :[]
    (int 0)) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_10() {
	let code = "foo.m += raise bar rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :m) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_11() {
	let code = "foo::m += raise bar rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :m) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_12() {
	let code = "foo.C += raise bar rescue nil";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :C) :+
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12402_13() {
	let code = "foo::C ||= raise bar rescue nil";
	let sexp = r##"
(or-asgn
  (casgn
    (lvar :foo) :C)
  (rescue
    (send nil :raise
      (lvar :bar))
    (resbody nil nil
      (nil)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_and() {
	let code = "foo and bar";
	let sexp = r##"
(and
  (lvar :foo)
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_and_1() {
	let code = "foo && bar";
	let sexp = r##"
(and
  (lvar :foo)
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn() {
	let code = "foo.a += 1";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :a) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_1() {
	let code = "foo::a += 1";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :a) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_2() {
	let code = "foo.A += 1";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :A) :+
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_string_concat() {
	let code = "\"foo#@a\" \"bar\"";
	let sexp = r##"
(dstr
  (dstr
    (str "foo")
    (ivar :@a))
  (str "bar"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self_block() {
	let code = "fun { }";
	let sexp = r##"
(block
  (send nil :fun)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self_block_1() {
	let code = "fun() { }";
	let sexp = r##"
(block
  (send nil :fun)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self_block_2() {
	let code = "fun(1) { }";
	let sexp = r##"
(block
  (send nil :fun
    (int 1))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_self_block_3() {
	let code = "fun do end";
	let sexp = r##"
(block
  (send nil :fun)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda() {
	let code = "->{ }";
	let sexp = r##"
(block
  (lambda)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_1() {
	let code = "-> * { }";
	let sexp = r##"
(block
  (lambda)
  (args
    (restarg)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_lambda_2() {
	let code = "-> do end";
	let sexp = r##"
(block
  (lambda)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_unscoped() {
	let code = "Foo";
	let sexp = r##"
(const nil :Foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
#[cfg(feature = "regex")]
fn parse_lvar_injecting_match() {
	let code = "/(?<match>bar)/ =~ 'bar'; match";
	let sexp = r##"
(begin
  (match-with-lvasgn
    (regexp
      (str "(?<match>bar)")
      (regopt))
    (str "bar"))
  (lvar :match))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_alias() {
	let code = "alias :foo bar";
	let sexp = r##"
(alias
  (sym :foo)
  (sym :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_while_post() {
	let code = "begin meth end while foo";
	let sexp = r##"
(while-post
  (lvar :foo)
  (kwbegin
    (send nil :meth)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_heredoc() {
	let code = "<<HERE\nfoo\nbar\nHERE";
	let sexp = r##"
(dstr
  (str "foo\n")
  (str "bar\n"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_heredoc_1() {
	let code = "<<'HERE'\nfoo\nbar\nHERE";
	let sexp = r##"
(dstr
  (str "foo\n")
  (str "bar\n"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_heredoc_2() {
	let code = "<<`HERE`\nfoo\nbar\nHERE";
	let sexp = r##"
(xstr
  (str "foo\n")
  (str "bar\n"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_int_line_() {
	let code = "__LINE__";
	let sexp = r##"
(int 1)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_symbol_plain() {
	let code = ":foo";
	let sexp = r##"
(sym :foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_symbol_plain_1() {
	let code = ":'foo'";
	let sexp = r##"
(sym :foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc() {
	let code = "p <<~E\nE";
	let sexp = r##"
(send nil :p
  (dstr))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_1() {
	let code = "p <<~E\n  E";
	let sexp = r##"
(send nil :p
  (dstr))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_2() {
	let code = "p <<~E\n  x\nE";
	let sexp = r##"
(send nil :p
  (str "x\n"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_3() {
	let code = "p <<~E\n  x\n    y\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "x\n")
    (str "  y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_4() {
	let code = "p <<~E\n\tx\n    y\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "x\n")
    (str "y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_5() {
	let code = "p <<~E\n\tx\n        y\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "x\n")
    (str "y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_6() {
	let code = "p <<~E\n    \tx\n        y\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "x\n")
    (str "y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_7() {
	let code = "p <<~E\n        \tx\n\ty\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "\tx\n")
    (str "y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_8() {
	let code = "p <<~E\n  x\n\ny\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "  x\n")
    (str "\n")
    (str "y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_9() {
	let code = "p <<~E\n  x\n    \n  y\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "x\n")
    (str "  \n")
    (str "y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_10() {
	let code = "p <<~E\n    x\n  \\  y\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "  x\n")
    (str "  y\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_11() {
	let code = "p <<~E\n    x\n  \\\ty\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "  x\n")
    (str "\ty\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_12() {
	let code = "p <<~\"E\"\n    x\n  #{foo}\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "  x\n")
    (str "")
    (begin
      (lvar :foo))
    (str "\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_13() {
	let code = "p <<~`E`\n    x\n  #{foo}\nE";
	let sexp = r##"
(send nil :p
  (xstr
    (str "  x\n")
    (str "")
    (begin
      (lvar :foo))
    (str "\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_dedenting_heredoc_14() {
	let code = "p <<~\"E\"\n    x\n  #{\"  y\"}\nE";
	let sexp = r##"
(send nil :p
  (dstr
    (str "  x\n")
    (str "")
    (begin
      (str "  y"))
    (str "\n")))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_false() {
	let code = "false";
	let sexp = r##"
(false)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if() {
	let code = "if foo then bar; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_1() {
	let code = "if foo; bar; end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_regex_interp() {
	let code = "/foo#{bar}baz/";
	let sexp = r##"
(regexp
  (str "foo")
  (begin
    (lvar :bar))
  (str "baz")
  (regopt))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_while_not_parens_do() {
	let code = "while not (true) do end";
	let sexp = r##"
(while
  (send
    (begin
      (true)) :!) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_string_dvar() {
	let code = "\"#@a #@@a #$a\"";
	let sexp = r##"
(dstr
  (ivar :@a)
  (str " ")
  (cvar :@@a)
  (str " ")
  (gvar :$a))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_cond_iflipflop() {
	let code = "if foo..bar; end";
	let sexp = r##"
(if
  (iflipflop
    (lvar :foo)
    (lvar :bar)) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_label() {
	let code = "def foo() a:b end";
	let sexp = r##"
(def :foo
  (args)
  (send nil :a
    (sym :b)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_label_1() {
	let code = "def foo\n a:b end";
	let sexp = r##"
(def :foo
  (args)
  (send nil :a
    (sym :b)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_arg_label_2() {
	let code = "f { || a:b }";
	let sexp = r##"
(block
  (send nil :f)
  (args)
  (send nil :a
    (sym :b)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_class_super_label() {
	let code = "class Foo < a:b; end";
	let sexp = r##"
(class
  (const nil :Foo)
  (send nil :a
    (sym :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_cmd() {
	let code = "foo.a += m foo";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :a) :+
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_cmd_1() {
	let code = "foo::a += m foo";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :a) :+
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_cmd_2() {
	let code = "foo.A += m foo";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :A) :+
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_cmd_3() {
	let code = "foo::A += m foo";
	let sexp = r##"
(op-asgn
  (casgn
    (lvar :foo) :A) :+
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_casgn_scoped() {
	let code = "Bar::Foo = 10";
	let sexp = r##"
(casgn
  (const nil :Bar) :Foo
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bang_cmd() {
	let code = "!m foo";
	let sexp = r##"
(send
  (send nil :m
    (lvar :foo)) :!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_complex() {
	let code = "42i";
	let sexp = r##"
(complex 42)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_complex_1() {
	let code = "42ri";
	let sexp = r##"
(complex 42)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_complex_2() {
	let code = "42.1i";
	let sexp = r##"
(complex 42.1)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_complex_3() {
	let code = "42.1ri";
	let sexp = r##"
(complex 42.1)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_nl_then() {
	let code = "if foo\nthen bar end";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ivasgn() {
	let code = "@var = 10";
	let sexp = r##"
(ivasgn :@var
  (int 10))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_optarg() {
	let code = "def f foo = 1; end";
	let sexp = r##"
(def :f
  (args
    (optarg :foo
      (int 1))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_optarg_1() {
	let code = "def f(foo=1, bar=2); end";
	let sexp = r##"
(def :f
  (args
    (optarg :foo
      (int 1))
    (optarg :bar
      (int 2))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_send_op_asgn_conditional() {
	let code = "a&.b &&= 1";
	let sexp = r##"
(and-asgn
  (csend
    (send nil :a) :b)
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_6() {
	let code = "f{  }";
	let sexp = r##"
(block
  (send nil :f)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_7() {
	let code = "f{ | | }";
	let sexp = r##"
(block
  (send nil :f)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_8() {
	let code = "f{ |;a| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (shadowarg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_9() {
	let code = "f{ |;\na\n| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (shadowarg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_10() {
	let code = "f{ || }";
	let sexp = r##"
(block
  (send nil :f)
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_11() {
	let code = "f{ |a| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (procarg0 :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_12() {
	let code = "f{ |a, c| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (arg :c)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_13() {
	let code = "f{ |a,| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_14() {
	let code = "f{ |a, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_15() {
	let code = "f{ |a, *s, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (restarg :s)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_16() {
	let code = "f{ |a, *, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (restarg)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_17() {
	let code = "f{ |a, *s| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (restarg :s)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_18() {
	let code = "f{ |a, *| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (restarg)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_19() {
	let code = "f{ |*s, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (restarg :s)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_20() {
	let code = "f{ |*, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (restarg)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_21() {
	let code = "f{ |*s| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (restarg :s)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_22() {
	let code = "f{ |*| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (restarg)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_23() {
	let code = "f{ |&b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_24() {
	let code = "f{ |a, o=1, o1=2, *r, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (optarg :o1
      (int 2))
    (restarg :r)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_25() {
	let code = "f{ |a, o=1, *r, p, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_26() {
	let code = "f{ |a, o=1, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_27() {
	let code = "f{ |a, o=1, p, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (optarg :o
      (int 1))
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_28() {
	let code = "f{ |a, *r, p, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (arg :a)
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_29() {
	let code = "f{ |o=1, *r, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (optarg :o
      (int 1))
    (restarg :r)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_30() {
	let code = "f{ |o=1, *r, p, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (optarg :o
      (int 1))
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_31() {
	let code = "f{ |o=1, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (optarg :o
      (int 1))
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_32() {
	let code = "f{ |o=1, p, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (optarg :o
      (int 1))
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn assert_parses_blockargs_33() {
	let code = "f{ |*r, p, &b| }";
	let sexp = r##"
(block
  (send nil :f)
  (args
    (restarg :r)
    (arg :p)
    (blockarg :b)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_not() {
	let code = "not foo";
	let sexp = r##"
(send
  (lvar :foo) :!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_not_1() {
	let code = "not(foo)";
	let sexp = r##"
(send
  (lvar :foo) :!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_not_2() {
	let code = "not()";
	let sexp = r##"
(send
  (begin) :!)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_nth_ref() {
	let code = "$10";
	let sexp = r##"
(nth-ref 10)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_unless() {
	let code = "unless foo then bar; end";
	let sexp = r##"
(if
  (lvar :foo) nil
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_unless_1() {
	let code = "unless foo; bar; end";
	let sexp = r##"
(if
  (lvar :foo) nil
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_var_op_asgn_cmd() {
	let code = "foo += m foo";
	let sexp = r##"
(op-asgn
  (lvasgn :foo) :+
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_yield() {
	let code = "yield(foo)";
	let sexp = r##"
(yield
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_yield_1() {
	let code = "yield foo";
	let sexp = r##"
(yield
  (lvar :foo))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_yield_2() {
	let code = "yield()";
	let sexp = r##"
(yield)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_yield_3() {
	let code = "yield";
	let sexp = r##"
(yield)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_else() {
	let code = "begin; meth; rescue; foo; else; bar; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody nil nil
      (lvar :foo))
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_regex_verification() {
	let code = "/#)/x";
	let sexp = r##"
(regexp
  (str "#)")
  (regopt :x))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_sclass() {
	let code = "class << foo; nil; end";
	let sexp = r##"
(sclass
  (lvar :foo)
  (nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_while_mod() {
	let code = "meth while foo";
	let sexp = r##"
(while
  (lvar :foo)
  (send nil :meth))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_cmdarg() {
	let code = "assert dogs";
	let sexp = r##"
(send nil :assert
  (send nil :dogs))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_cmdarg_1() {
	let code = "assert do: true";
	let sexp = r##"
(send nil :assert
  (hash
    (pair
      (sym :do)
      (true))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_cmdarg_2() {
	let code = "f x: -> do meth do end end";
	let sexp = r##"
(send nil :f
  (hash
    (pair
      (sym :x)
      (block
        (lambda)
        (args)
        (block
          (send nil :meth)
          (args) nil)))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_character() {
	let code = "?a";
	let sexp = r##"
(str "a")
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_class() {
	let code = "class Foo; end";
	let sexp = r##"
(class
  (const nil :Foo) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_class_1() {
	let code = "class Foo end";
	let sexp = r##"
(class
  (const nil :Foo) nil nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_masgn_cmd() {
	let code = "foo, bar = m foo";
	let sexp = r##"
(masgn
  (mlhs
    (lvasgn :foo)
    (lvasgn :bar))
  (send nil :m
    (lvar :foo)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_rescue_mod_asgn() {
	let code = "foo = meth rescue bar";
	let sexp = r##"
(lvasgn :foo
  (rescue
    (send nil :meth)
    (resbody nil nil
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_until() {
	let code = "until foo do meth end";
	let sexp = r##"
(until
  (lvar :foo)
  (send nil :meth))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_until_1() {
	let code = "until foo; meth end";
	let sexp = r##"
(until
  (lvar :foo)
  (send nil :meth))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_next_block() {
	let code = "next fun foo do end";
	let sexp = r##"
(next
  (block
    (send nil :fun
      (lvar :foo))
    (args) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_cmd() {
	let code = "fun (f bar)";
	let sexp = r##"
(send nil :fun
  (begin
    (send nil :f
      (lvar :bar))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_op_asgn_index() {
	let code = "foo[0, 1] += 2";
	let sexp = r##"
(op-asgn
  (send
    (lvar :foo) :[]
    (int 0)
    (int 1)) :+
  (int 2))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_string_interp() {
	let code = "\"foo#{bar}baz\"";
	let sexp = r##"
(dstr
  (str "foo")
  (begin
    (lvar :bar))
  (str "baz"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11989() {
	let code = "p <<~\"E\"\n  x\\n   y\nE";
	let sexp = r##"
(send nil :p
  (str "x\n   y\n"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ensure_empty() {
	let code = "begin ensure end";
	let sexp = r##"
(kwbegin
  (ensure nil nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12669() {
	let code = "a = b = raise :x";
	let sexp = r##"
(lvasgn :a
  (lvasgn :b
    (send nil :raise
      (sym :x))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12669_1() {
	let code = "a += b = raise :x";
	let sexp = r##"
(op-asgn
  (lvasgn :a) :+
  (lvasgn :b
    (send nil :raise
      (sym :x))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12669_2() {
	let code = "a = b += raise :x";
	let sexp = r##"
(lvasgn :a
  (op-asgn
    (lvasgn :b) :+
    (send nil :raise
      (sym :x))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_12669_3() {
	let code = "a += b += raise :x";
	let sexp = r##"
(op-asgn
  (lvasgn :a) :+
  (op-asgn
    (lvasgn :b) :+
    (send nil :raise
      (sym :x))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_case_cond_else() {
	let code = "case; when foo; 'foo'; else 'bar'; end";
	let sexp = r##"
(case nil
  (when
    (lvar :foo)
    (str "foo"))
  (str "bar"))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_alias_gvar() {
	let code = "alias $a $b";
	let sexp = r##"
(alias
  (gvar :$a)
  (gvar :$b))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_alias_gvar_1() {
	let code = "alias $a $+";
	let sexp = r##"
(alias
  (gvar :$a)
  (back-ref :$+))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_resbody_list() {
	let code = "begin; meth; rescue Exception; bar; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody
      (array
        (const nil :Exception)) nil
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_or() {
	let code = "foo or bar";
	let sexp = r##"
(or
  (lvar :foo)
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_or_1() {
	let code = "foo || bar";
	let sexp = r##"
(or
  (lvar :foo)
  (lvar :bar))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_const_scoped() {
	let code = "Bar::Foo";
	let sexp = r##"
(const
  (const nil :Bar) :Foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_lvar() {
	let code = "foo";
	let sexp = r##"
(lvar :foo)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_block() {
	let code = "fun () {}";
	let sexp = r##"
(block
  (send nil :fun
    (begin))
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_assocs_comma() {
	let code = "foo[:baz => 1,]";
	let sexp = r##"
(send
  (lvar :foo) :[]
  (hash
    (pair
      (sym :baz)
      (int 1))))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_bug_do_block_in_cmdarg() {
	let code = "tap (proc do end)";
	let sexp = r##"
(send nil :tap
  (begin
    (block
      (send nil :proc)
      (args) nil)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_class_super() {
	let code = "class Foo < Bar; end";
	let sexp = r##"
(class
  (const nil :Foo)
  (const nil :Bar) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_regex_plain() {
	let code = "/source/im";
	let sexp = r##"
(regexp
  (str "source")
  (regopt :i :m))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_def() {
	let code = "def foo; end";
	let sexp = r##"
(def :foo
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_def_1() {
	let code = "def String; end";
	let sexp = r##"
(def :String
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_def_2() {
	let code = "def String=; end";
	let sexp = r##"
(def :String=
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_def_3() {
	let code = "def until; end";
	let sexp = r##"
(def :until
  (args) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_ruby_bug_11107() {
	let code = "p ->() do a() do end end";
	let sexp = r##"
(send nil :p
  (block
    (lambda)
    (args)
    (block
      (send nil :a)
      (args) nil)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_words_empty() {
	let code = "%w[]";
	let sexp = r##"
(array)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_array_words_empty_1() {
	let code = "%W()";
	let sexp = r##"
(array)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_space_args_hash_literal_then_block() {
	let code = "f 1, {1 => 2} {1}";
	let sexp = r##"
(block
  (send nil :f
    (int 1)
    (hash
      (pair
        (int 1)
        (int 2))))
  (args)
  (int 1))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_if_mod() {
	let code = "bar if foo";
	let sexp = r##"
(if
  (lvar :foo)
  (lvar :bar) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_args_block_pass() {
	let code = "fun(&bar)";
	let sexp = r##"
(send nil :fun
  (block-pass
    (lvar :bar)))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_kwoptarg() {
	let code = "def f(foo: 1); end";
	let sexp = r##"
(def :f
  (args
    (kwoptarg :foo
      (int 1))) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_resbody_list_mrhs() {
	let code = "begin; meth; rescue Exception, foo; bar; end";
	let sexp = r##"
(kwbegin
  (rescue
    (send nil :meth)
    (resbody
      (array
        (const nil :Exception)
        (lvar :foo)) nil
      (lvar :bar)) nil))
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

#[test]
fn parse_when_then() {
	let code = "case foo; when 'bar' then bar; end";
	let sexp = r##"
(case
  (lvar :foo)
  (when
    (str "bar")
    (lvar :bar)) nil)
"##;
	parse_and_cmp!(code, sexp, OPTIONS);
}

