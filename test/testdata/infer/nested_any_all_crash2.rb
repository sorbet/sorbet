# typed: true

class A; end

class Parent; end
class Child < Parent; end

module IFoo; end
class Foo
  include IFoo
end

module Main
  extend T::Sig

  sig {params(x: T.all(T.any(A, Parent), IFoo, T.any(Foo, Child))).void}
  def self.test(x)
    T.reveal_type(x) # error: `T.all(IFoo, T.any(A, Parent), T.any(Foo, Child))`
  end
end
