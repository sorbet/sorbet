# typed: true

class Suit < T::Enum
  enums do
    Spades = new('SPADES')
    Hearts = new('HEARTS')
    Clubs = new('CLUBS')
    Diamonds = new('DIAMONDS')
  end
end