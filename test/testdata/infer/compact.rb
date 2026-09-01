# typed: true

T.assert_type!(
  [1, nil].compact,
  T::Array[Integer],
)

T.assert_type!(
  [1, 2, 3].compact,
  T::Array[Integer],
)

T.assert_type!(
  [[1], nil].compact,
  T::Array[[Integer]],
)

extend T::Sig

sig {returns(T.nilable([String, String]))}
def maybe_row
  T.unsafe(nil)
end

sig {returns(T::Array[[String, String]])}
def compact_tuple_unions
  [
    T.unsafe(nil) ? ["Payment method", T.unsafe(nil)] : nil,
    maybe_row,
    T.unsafe(nil) ? ["Card verification", T.unsafe(nil)] : nil,
  ].compact
end
