# typed: true

class A; end
class B; end
module M; end

module Foo
  extend T::Sig
  extend T::Helpers

  interface!

  sig {abstract.void}
  def foo; end
end

class X1
  extend T::Sig
  include Foo
  sig {override.void}
  def foo; end
end

class Y < B; end

class Z
  extend T::Sig

  include M
  include Foo

  sig {override.void}
  def foo; end
end

module Main
  extend T::Sig

  TypeArg = T.type_alias {T.all(T.any(A, B, M), Foo, T.any(X1, Y, Z))}

  sig {params(x: TypeArg).void}
  def self.test(x)
    x.foo
  end
end

