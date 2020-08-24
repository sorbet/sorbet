# typed: true
def foo
  untyped = T.let(42, T.untyped)

  # Literal ranges

  lr1 = 1..2
  T.reveal_type(lr1) # error: Revealed type: `T::Range[Integer]`

  lr2 = 1...2
  T.reveal_type(lr2) # error: Revealed type: `T::Range[Integer]`

  lr3 = 1..nil
  T.reveal_type(lr3) # error: Revealed type: `T::Range[Integer]`

  lr4 = 1...nil
  T.reveal_type(lr4) # error: Revealed type: `T::Range[Integer]`

  lr5 = (1..)
  T.reveal_type(lr5) # error: Revealed type: `T::Range[Integer]`

  lr6 = (1...)
  T.reveal_type(lr6) # error: Revealed type: `T::Range[Integer]`

  lr7 = (1.."a")
  T.reveal_type(lr7) # error: Revealed type: `T::Range[T.untyped]`

  lr8 = nil..42
  T.reveal_type(lr8) # error: Revealed type: `T::Range[Integer]`

  lr9 = nil...42
  T.reveal_type(lr9) # error: Revealed type: `T::Range[Integer]`

  lr10 = (nil..)
  T.reveal_type(lr10) # error: Revealed type: `T::Range[T.untyped]`

  lr11 = (nil...)
  T.reveal_type(lr11) # error: Revealed type: `T::Range[T.untyped]`

  lr12 = nil..nil
  T.reveal_type(lr12) # error: Revealed type: `T::Range[T.untyped]`

  lr13 = nil...nil
  T.reveal_type(lr13) # error: Revealed type: `T::Range[T.untyped]`

  lr14 = untyped..42
  T.reveal_type(lr14) # error: Revealed type: `T::Range[T.untyped]`

  lr15 = 1..untyped
  T.reveal_type(lr15) # error: Revealed type: `T::Range[T.untyped]`

  lr16 = untyped..untyped
  T.reveal_type(lr16) # error: Revealed type: `T::Range[T.untyped]`

  lr_first = (1..42).first
  T.reveal_type(lr_first) # error: Revealed type: `Integer`

  lr_last = ('a'..'z').last
  T.reveal_type(lr_last) # error: Revealed type: `String`

  # Range.new

  rn1 = Range.new # error: Not enough arguments provided for method `Range#initialize`. Expected: `2..3`, got: `0`
  T.reveal_type(rn1) # error: Revealed type: `T::Range[T.untyped]`

  rn2 = Range.new(nil) # error: Not enough arguments provided for method `Range#initialize`. Expected: `2..3`, got: `1`
  T.reveal_type(rn2) # error: Revealed type: `T::Range[T.untyped]`

  rn3 = Range.new(1, 2)
  T.reveal_type(rn3) # error: Revealed type: `T::Range[T.untyped]`

  rn4 = Range.new(1, 2, true)
  T.reveal_type(rn4) # error: Revealed type: `T::Range[T.untyped]`

  rn5 = Range.new(1, nil)
  T.reveal_type(rn5) # error: Revealed type: `T::Range[T.untyped]`

  rn6 = Range.new(1, true)
  T.reveal_type(rn6) # error: Revealed type: `T::Range[T.untyped]`

  rn7 = Range.new(nil, 42)
  T.reveal_type(rn7) # error: Revealed type: `T::Range[T.untyped]`

  rn8 = Range.new(nil, nil)
  T.reveal_type(rn8) # error: Revealed type: `T::Range[T.untyped]`

  rn9 = Range.new(1, 'c')
  T.reveal_type(rn9) # error: Revealed type: `T::Range[T.untyped]`

  rn10 = Range.new(untyped, 42)
  T.reveal_type(rn10) # error: Revealed type: `T::Range[T.untyped]`

  rn11 = Range.new(1, untyped)
  T.reveal_type(rn11) # error: Revealed type: `T::Range[T.untyped]`

  rn12 = Range.new(untyped, untyped)
  T.reveal_type(rn12) # error: Revealed type: `T::Range[T.untyped]`

  rn_first = Range.new(1, 42).first
  T.reveal_type(rn_first) # error: Revealed type: `T.untyped`

  rn_last = Range.new('a', 'z').last
  T.reveal_type(rn_last) # error: Revealed type: `T.untyped`

  # T::Range[].new

  tr1 = T::Range.new # error: Method `new` does not exist on `T.class_of(T::Range)`
  T.reveal_type(tr1) # error: Revealed type: `T.untyped`

  tr2 = T::Range[Integer].new # error: Not enough arguments provided for method `Range#initialize`. Expected: `2..3`, got: `0`
  T.reveal_type(tr2) # error: Revealed type: `T::Range[Integer]`

  tr3 = T::Range[Integer].new(1, 2)
  T.reveal_type(tr3) # error: Revealed type: `T::Range[Integer]`

  tr4 = T::Range[Integer].new(1, 2, true)
  T.reveal_type(tr4) # error: Revealed type: `T::Range[Integer]`

  tr5 = T::Range[Integer].new(1, nil)
  T.reveal_type(tr5) # error: Revealed type: `T::Range[Integer]`

  tr6 = T::Range[Integer].new(1, true) # error: Expected `T.nilable(Integer)` but found `TrueClass` for argument `_end`
  T.reveal_type(tr6) # error: Revealed type: `T::Range[Integer]`

  tr7 = T::Range[Integer].new(nil, 42) # error: Expected `Integer` but found `NilClass` for argument `_begin`
  T.reveal_type(tr7) # error: Revealed type: `T::Range[Integer]`

  tr8 = T::Range[Integer].new(nil, nil) # error: Expected `Integer` but found `NilClass` for argument `_begin`
  T.reveal_type(tr8) # error: Revealed type: `T::Range[Integer]`

  tr9 = T::Range[T.untyped].new(1, 'c')
  T.reveal_type(tr9) # error: Revealed type: `T::Range[T.untyped]`

  tr10 = T::Range[Integer].new(untyped, 42)
  T.reveal_type(tr10) # error: Revealed type: `T::Range[Integer]`

  tr11 = T::Range[Integer].new(1, untyped)
  T.reveal_type(tr11) # error: Revealed type: `T::Range[Integer]`

  tr12 = T::Range[Integer].new(untyped, untyped)
  T.reveal_type(tr12) # error: Revealed type: `T::Range[Integer]`

  tr_first = T::Range[Integer].new(1, 42).first
  T.reveal_type(tr_first) # error: Revealed type: `Integer`

  tr_last = T::Range[String].new('a', 'z').last
  T.reveal_type(tr_last) # error: Revealed type: `String`
end
