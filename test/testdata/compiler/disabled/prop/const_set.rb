# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  const :foo, Integer
end

a = A.new(foo: 1247)

# Raises at runtime.
# T.unsafe() to hide the fact that Sorbet can tell `foo=` doesn't exist.
T.unsafe(a).foo = 0
