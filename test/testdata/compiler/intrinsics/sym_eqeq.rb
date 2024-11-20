# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(sym: Symbol, obj: T.untyped).returns(T::Boolean)}
def symeq(sym, obj)
  sym == obj
end


p symeq(:sym, 1)
p symeq(:sym, :sym)
p symeq(:sym, nil)
