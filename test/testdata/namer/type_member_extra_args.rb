# typed: true

class A
  extend T::Sig
  extend T::Generic

  X = type_member(:out, :extra)
#                       ^^^^^^ error: Too many arguments in `type_member` definition for `X`
  Y = type_template(:out, :extra)
#                         ^^^^^^ error: Too many arguments in `type_template` definition for `Y`

  sig {params(x: X).void}
  def foo(x)
    T.reveal_type(x) # error: Revealed type: `T.untyped`
  end
end
