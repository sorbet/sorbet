# typed: true

class Upper
  extend T::Sig, T::Generic
  abstract!

  X = type_member { {upper: Numeric} }
# ^ def: Upper::X

  sig { abstract.returns(X) }
  #                      ^ usage: Upper::X
  def foo; end
end

class Fixed
  extend T::Sig, T::Generic
  abstract!

  X = type_member { {fixed: Integer} }
# ^ def: Fixed::X

  sig { returns(X) }
  #             ^ usage: Fixed::X
  def foo; 0; end
end
