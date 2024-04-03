# compiled: true
# typed: strict

class Default < T::Struct
  prop :foo, Integer, default: 3
end

T.reveal_type(Default.new.foo) # error: Revealed type: `Integer`
Default.new(foo: "no")
#                ^^^^ error: Expected `Integer` but found `String("no")` for argument `foo`
Default.new(foo: 3, bar: 4)
#                   ^^^^^^ error: Unrecognized keyword argument `bar` passed for method `Default#initialize`
T.reveal_type(Default.new(foo: 3).foo) # error: Revealed type: `Integer`

class Bad < T::Struct
  prop :no_type_on_prop  # error: Not enough arguments provided for method `T::Props::ClassMethods#prop`
  const :no_type_on_const  # error: Not enough arguments provided for method `T::Props::ClassMethods#const`
end
