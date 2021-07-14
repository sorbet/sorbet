# typed: true
extend T::Sig

class StructWithTuples < T::Struct
  prop :unary_tuple, [Symbol]
  prop :tuple, [Integer, String]
  prop :triple, [Float, TrueClass, FalseClass]
  prop :array_of_tuple, T::Array[[Integer, String]]
  prop :tuple_with_combinators, [T.nilable(String), T.class_of(Integer)]
  prop :combinator_with_tuple, T.nilable([Symbol, Float])
end

class StructWithBadTuples < T::Struct
  prop :bad_literal, [42, 42]
end

x = StructWithTuples.new(
  unary_tuple: [:hello],
  tuple: [0, ''],
  triple: [0.0, true, false],
  array_of_tuple: [[0, '']],
  tuple_with_combinators: [nil, Integer]
)

T.reveal_type(x.unary_tuple) # error: `[Symbol] (1-tuple)`
T.reveal_type(x.tuple) # error: `[Integer, String] (2-tuple)`
T.reveal_type(x.triple) # error: `[Float, TrueClass, FalseClass] (3-tuple)`
T.reveal_type(x.array_of_tuple) # error: `T::Array[[Integer, String]]`
T.reveal_type(x.tuple_with_combinators) # error: `[T.nilable(String), T.class_of(Integer)] (2-tuple)`
T.reveal_type(x.combinator_with_tuple) # error: `T.nilable([Symbol, Float])`
