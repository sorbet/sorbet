# typed: true
extend T::Sig

TYPES = T.let(
  [Integer, String],
  [T.class_of(Integer), T.class_of(String)]
)

def foo(x)
  case x
  when *[Integer, String]
    T.reveal_type(x) # error: `T.any(Integer, String)`
  end

  case x
  when *TYPES
    T.reveal_type(x) # error: `T.any(Integer, String)`
  end

  case x
  when *[]
    T.reveal_type(x) # error: This code is unreachable
  end

  case x
  when *[Integer]
    T.reveal_type(x) # error: `Integer`
  end
end

sig { params(int: Integer, int_or_str: T.any(Integer, String), sym: Symbol).void }
def example(int, int_or_str, sym)
  case int_or_str
  when *[Integer]
    T.reveal_type(int_or_str) # error: `Integer`
  else
    T.reveal_type(int_or_str) # error: `String`
  end

  case int_or_str
  when *TYPES
    T.reveal_type(int_or_str) # error: `T.any(Integer, String)`
  else
    T.absurd(int_or_str)
    T.reveal_type(int_or_str) # error: This code is unreachable
  end

  x = case int_or_str
  when *TYPES
    T.reveal_type(int_or_str) # error: `T.any(Integer, String)`
    'initialized'
  end
  T.reveal_type(x) # error: `String("initialized")`

  case sym
  when *TYPES
    T.reveal_type(sym) # error: This code is unreachable
  else
    T.reveal_type(sym) # error: `Symbol`
  end

  untyped = T.unsafe(nil)
  case untyped
  when *[nil, true]
    T.reveal_type(untyped) # error: `T.nilable(TrueClass)`
  end
end

class Suit < T::Enum
  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end
end

RedSuits = T.let(
  [Suit::Hearts, Suit::Diamonds].freeze,
  [Suit::Hearts, Suit::Diamonds]
)

sig { params(suit: Suit).void }
def enum_example(suit)
  case suit
  when *RedSuits
    T.reveal_type(suit) # error: `T.any(Suit::Hearts, Suit::Diamonds)`
  else
    T.reveal_type(suit) # error: `T.any(Suit::Clubs, Suit::Spades)`
  end

  case suit
  when *RedSuits
    T.reveal_type(suit) # error: `T.any(Suit::Hearts, Suit::Diamonds)`
  when *[Suit::Clubs, Suit::Spades]
    nil
  else
    T.absurd(suit)
    T.reveal_type(suit) # error: This code is unreachable
  end

  bool = case suit
  when *RedSuits
    T.reveal_type(suit) # error: `T.any(Suit::Hearts, Suit::Diamonds)`
    true
  when *[Suit::Clubs, Suit::Spades]
    false
  end
  T.reveal_type(bool) # error: `T::Boolean`
end
