# typed: true

module M
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  A = type_member(:in)

  interface!

  sig {abstract.params(x: A).void}
  def foo(x); end

  module_function :foo
end

T.reveal_type(M.foo) # error: Revealed type: `T.untyped`
