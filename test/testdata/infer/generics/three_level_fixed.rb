# typed: strict

class Example
  extend T::Sig
  extend T::Generic
  Z = type_member { {fixed: Integer} }
  Y = type_member { {fixed: Z} }
  X = type_member { {fixed: Y} }

  sig { returns(X) }
  def returns_int
    0
  end

  sig { returns(X) }
  def returns_string
    '' # error: Expected `Integer` but found `String("")` for method result type
  end

  sig { params(x: X).void }
  def accepts_int(x)
    T.reveal_type(x) # error: `Integer`
  end

  sig { void }
  def example
    accepts_int(returns_int)
    accepts_int(0)
    accepts_int('') # error: Expected `Integer` but found `String("")` for argument `x`
  end
end
