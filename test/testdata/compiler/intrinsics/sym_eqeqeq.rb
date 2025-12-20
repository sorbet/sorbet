# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

sig {params(sym: Symbol, obj: T.untyped).returns(T::Boolean)}
def symeq(sym, obj)
  sym === obj
end

# INITIAL-LABEL: "func_Object#5symeq"
# INITIAL: call i64 @sorbet_rb_sym_equal
# INITIAL{LITERAL}: }

p symeq(:sym, 1)
p symeq(:sym, :sym)
p symeq(:sym, nil)
