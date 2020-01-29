# typed: true

class A
  extend T::Sig

  sig { params(i: Integer).void }
  def foo(i); end
end

A.new.foo(42)
A.new.foo("foo")
