# typed: true

class A < T::Struct
  const :foo, Integer

  attr_reader :bar

  def qux; end
end

A.new(foo: 0).foo = 1
A.new(foo: 0).bar = 1
A.new(foo: 0).qux = 1
