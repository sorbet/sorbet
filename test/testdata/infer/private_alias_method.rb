# typed: true

class A < T::Struct
  const :foo, Integer
  alias_method :foo_DEPRECATED_BUT_PUBLIC, :foo
  private :foo
end

A.new(foo: 0).foo_DEPRECATED_BUT_PUBLIC
A.new(foo: 0).foo # error: Non-private call to private method `A#foo`
