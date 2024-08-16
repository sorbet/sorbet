# typed: true

class Suit < T::Enum
  extend T::Sig

  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end

  Reds = T.let([Hearts, Diamonds], T::Array[Suit])
  Red = T.type_alias { T.any(Hearts, Diamonds) }

  sig { returns(T.nilable(Red)) }
  def to_red
    case self
    when Spades, Clubs
      nil
    else
      self
    end
  end
end
