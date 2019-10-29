# typed: true

module Foo
  extend T::Helpers
  extend T::Sig
  interface!

  sig {abstract.params(a: Integer).void}
  def foo(a=1)
  end
end
