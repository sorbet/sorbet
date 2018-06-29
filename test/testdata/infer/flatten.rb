# typed: true

T.assert_type!(
  [1].flatten,
  T::Array[Integer],
)

T.assert_type!(
  [[1]].flatten,
  T::Array[Integer],
)

T.assert_type!(
  [1, [2], [3], [[3], [[4]]]].flatten,
  T::Array[Integer],
)
