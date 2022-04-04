# typed: strict

class SomeDefault < T::Struct
  prop :foo, Integer
  prop :bar, T::Boolean, default: false
end

SomeDefault.new(foo: 1)
SomeDefault.new(foo: 2, bar: true)
SomeDefault.new(foo: 3, bar: false)
SomeDefault.new
#              ^ error: Missing required keyword argument `foo` for method `SomeDefault#initialize`
