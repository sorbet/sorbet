# typed: strict

# Discovered in https://github.com/sorbet/sorbet/issues/7969

class A
  extend T::Sig
  sig { params(x: Integer, x: String).void }
                         # ^ error: Hash key `x` is duplicated
                         # ^ error: Unknown parameter name `x`
  def initialize(x:, x: nil)
                   # ^^ error: Malformed `sig`
                   # ^^^^^^ error: duplicate argument name x
    @x = T.let(x, T.nilable(String))
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Argument does not have asserted type
    @x = T.let(x, T.nilable(String))
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Argument does not have asserted type
  end
end
