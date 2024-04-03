# compiled: true
# typed: strict

class Root < ::T::Struct
  prop :foo, Integer
end

Root.new # error: Missing required keyword argument `foo` for method `Root#initialize`
Root.new(foo: "no")
#             ^^^^ error: Expected `Integer` but found `String("no")` for argument `foo`
Root.new(foo: 3, bar: 4)
#                ^^^^^^ error: Unrecognized keyword argument `bar` passed for method `Root#initialize`
T.reveal_type(Root.new(foo: 3).foo) # error: Revealed type: `Integer`
