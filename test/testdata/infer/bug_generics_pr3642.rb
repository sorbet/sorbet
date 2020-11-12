# frozen_string_literal: true
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
  T.proc.params(x: T.any(String, Integer)).void
)

foo(f, 0)
