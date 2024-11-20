# typed: strict

class Normal < T::Struct
  prop :foo, Integer
end

Normal.new
#         ^ error: Missing required keyword argument `foo` for method `Normal#initialize`
Normal.new(foo: "no")
#               ^^^^ error: Expected `Integer` but found `String("no")` for argument `foo`
Normal.new(foo: 3, bar: 4)
#                  ^^^^^^ error: Unrecognized keyword argument `bar` passed for method `Normal#initialize`
T.reveal_type(Normal.new(foo: 3).foo) # error: Revealed type: `Integer`
