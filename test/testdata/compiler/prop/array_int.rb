# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  const :foo, T::Array[Integer]
end

a1 = A.new(foo: [42])
p a1
p a1.foo
