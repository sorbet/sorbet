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

  sig { params(x: T.all(T.any(A, Parent), T.any(Foo, Child))).void }
  def self.pretest(x)
    T.reveal_type(x) # error: `Child`

    # This is kind of nuts. It boils down to this:
    #   (A & Foo)  |  (A & Child) | (Parent & Foo) | (Parent & Child)
    # and that's just:
    #   T.noreturn |  T.noreturn  |   T.noreturn   |      Child
  end

  sig {params(x: T.all(T.any(A, Parent), IFoo, T.any(Foo, Child))).void}
  def self.test(x)
    T.reveal_type(x) # error: `T.all(IFoo, Child)`
  end
end
