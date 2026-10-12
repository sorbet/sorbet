# typed: true
extend T::Sig

class Suit < T::Enum
  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end
end

a = T.let(nil, T.nilable(Suit))

case a
when Suit::Spades
when nil, Suit
else
  T.absurd(a)
end
