# frozen_string_literal: true
# typed: strict
# compiled: true

class Inexact < T::InexactStruct
  prop :foo, Integer
  prop :bar, String
end

inexact = Inexact.new(foo: 3, bar: "hey")
p inexact

p inexact.foo
p inexact.bar

inexact.foo = 789
inexact.bar = 'yeh'

p inexact

class Child < Inexact
  prop :qux, Symbol
end

child = Child.new(foo: 141, bar: "hello", qux: :my_symbol)
p child

p child.qux
