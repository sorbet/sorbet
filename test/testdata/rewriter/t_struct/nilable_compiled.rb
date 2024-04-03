# compiled: true
# typed: strict

class Nilable < T::Struct
  prop :foo, T.nilable(Integer)
end

T.reveal_type(Nilable.new.foo) # error: Revealed type: `T.nilable(Integer)`
Nilable.new(foo: "no")
#                ^^^^ error: Expected `T.nilable(Integer)` but found `String("no")` for argument `foo`
  Nilable.new(foo: 3, bar: 4)
  #                   ^^^^^^ error: Unrecognized keyword argument `bar` passed for method `Nilable#initialize`
T.reveal_type(Nilable.new(foo: 3).foo) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type(Nilable.new(foo: nil).foo) # error: Revealed type: `T.nilable(Integer)`
