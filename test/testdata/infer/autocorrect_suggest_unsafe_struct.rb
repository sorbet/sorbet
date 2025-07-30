# typed: true
# enable-suggest-unsafe: true

class S < T::Struct
  prop :foo, String
  prop :bar, Integer
end

# Missing required keyword arguments
S.new
S.new(foo: "hello")
S.new(bar: 42)

# This should work fine (no error)
S.new(foo: "hello", bar: 42)