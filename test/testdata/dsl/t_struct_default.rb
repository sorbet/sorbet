# typed: true

class Default < T::Struct
  prop :foo, Integer, default: 3
end

T.reveal_type(Default.new.foo) # error: Revealed type: `Integer`
Default.new(foo: "no") # error: Expected `Integer` but found `String("no")` for argument `foo`
Default.new(foo: 3, bar: 4) # error: Unrecognized keyword argument `bar` passed for method `Default#initialize`
T.reveal_type(Default.new(foo: 3).foo) # error: Revealed type: `Integer`
