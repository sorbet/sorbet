# typed: strict
# frozen_string_literal: true

int_range = (1..3)
int_array = [4, 5]
mixed_array = [6.0, 7]

T.assert_type!(int_range.chain(int_array), T::Enumerator[Integer])
T.assert_type!(
  int_range.chain(mixed_array),
  T::Enumerator[T.any(Integer, Float)]
)

T.assert_type!(
  int_array.permutation + mixed_array.permutation,
  T::Array[T.any(Float, Integer)]
)
