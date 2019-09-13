# typed: true

# We currently desugar module_function in a DSL pass, but we
# previously treated it specially in the namer, and there were
# edge-cases with generics where we'd treat the static and instance
# method as being basically "the same", but this would cause errors
# because of the differing way we represent generic instances versus
# generic classes. This shouldn't be a problem with the DSL-ified
# version.

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
