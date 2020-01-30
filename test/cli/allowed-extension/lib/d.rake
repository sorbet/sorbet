# typed: true

class D
  extend T::Sig

  sig { params(i: Integer).void }
  def foo(i); end
end

D.new.foo(42)
D.new.foo("foo")
