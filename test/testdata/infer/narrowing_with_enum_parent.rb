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

sig {params(cond: T::Boolean).void}
def test_lub_with_enum_parent(cond)
  parent = T.let(Suit::Spades, Suit)
  variants = T.must([Suit::Hearts, Suit::Clubs].sample)

  left = cond ? parent : variants
  T.reveal_type(left) # error: Revealed type: `Suit`

  right = cond ? variants : parent
  T.reveal_type(right) # error: Revealed type: `Suit`
end

sig do
  type_parameters(:U)
    .params(value: T.all(T.type_parameter(:U), T.any(Suit::Spades, Suit::Hearts)))
    .void
end
def test_enum_union_intersection(value)
  T.reveal_type(value) # error: Revealed type: `T.all(T.type_parameter(:U) (of Object#test_enum_union_intersection), T.any(Suit::Spades, Suit::Hearts))`
end
