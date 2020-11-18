# typed: strict
extend T::Sig

sig do
  type_parameters(:U)
    .params(
      f: T.proc.params(x: T.type_parameter(:U)).void,
      x: T.type_parameter(:U)
    )
    .void
end
def foo(f, x)
end

f = T.let(
  proc {|x| nil},
  T.proc.params(x: String).void
)

# These constraints must be true:
#
#   T.type_parameter(:U) <: String
#   Integer <: T.type_parameter(:U)
#
# but they can't be.
foo(f, 0) # error: Could not find valid instantiation of type parameters
