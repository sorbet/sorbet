# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  const :simple, Integer
end

class B < T::Struct
  const :with_default, String, default: 'Hello, world!'
  const :implicit_nil_default, T.nilable(Integer)
end

a = A.new(simple: 1247)
p a.simple

b = B.new
p b.with_default
p b.implicit_nil_default
