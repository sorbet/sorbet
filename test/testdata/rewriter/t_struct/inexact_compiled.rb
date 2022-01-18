# compiled: true
# typed: strict

class Inexact < T::InexactStruct
  prop :foo, Integer
  prop :bar, String
end

Inexact.new
Inexact.new(foo: 3, bar: "hey")
Inexact.new(foo: false, quz: [1, 2, 5])

class Child < Inexact
  prop :qux, Symbol
end
