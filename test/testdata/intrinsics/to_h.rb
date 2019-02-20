# typed: true

T.assert_type!(
  [].to_h,
  T::Hash[T.untyped, T.untyped]
)

T.assert_type!(
  [[:a, 1], [:b, 2]].to_h,
  T::Hash[Symbol, Integer]
)

["hi"].to_h # error: doesn't match `T::Enumerable[[U, V]]`

T.assert_type!(
  T.cast([], T::Enumerable[[String, Symbol]]).to_h,
  T::Hash[String, Symbol],
)
