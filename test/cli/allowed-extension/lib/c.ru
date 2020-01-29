# typed: true

class C
  extend T::Sig

  sig { params(i: Integer).void }
  def foo(i); end
end

C.new.foo(42)
C.new.foo("foo")
