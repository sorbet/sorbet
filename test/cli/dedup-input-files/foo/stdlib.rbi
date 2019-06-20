# typed: true

class A
end

class Foo
  extend T::Sig

  sig { params(a: A).void }
  def foo(a); end
end
