# typed: true
extend T::Sig

class Suit < T::Enum
  enums do
    Hearts = new
    Diamonds = new
    Clubs = new
    Spades = new
  end
end

class Color < T::Enum
  enums do
    Red = new
    Blue = new
    Green = new
  end
end

sig {params(cond: T::Boolean).void}
def test_different_enum_lub(cond)
  x = if cond
    res = [Suit::Hearts, Suit::Diamonds].sample
    T.must(res)
  else
    res = [Color::Red, Color::Blue].sample
    T.must(res)
  end

  T.reveal_type(x) # error: Revealed type: `T.any(Suit::Hearts, Suit::Diamonds, Color::Red, Color::Blue)`

  case x
  when Suit::Hearts, Suit::Diamonds
    T.reveal_type(x) # error: Revealed type: `T.any(Suit::Hearts, Suit::Diamonds)`
  when Color::Red, Color::Blue
    T.reveal_type(x) # error: Revealed type: `T.any(Color::Red, Color::Blue)`
  end
end
