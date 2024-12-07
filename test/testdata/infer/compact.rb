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

T.assert_type!(
  T.let({a: 1, b: nil, c: 3}, T::Hash[Symbol, T.nilable(Integer)]).compact,
  T::Hash[Symbol, Integer]
)

T.assert_type!(
  T.let({a: 1, b: 2, c: 3}, T::Hash[Symbol, Integer]).compact,
  T::Hash[Symbol, Integer]
)

T.assert_type!(
  T.let({}, T::Hash[T.untyped, T.untyped]).compact,
  T::Hash[T.untyped, T.untyped]
)

T.assert_type!(
  T.let({a: 1, b: "two", c: nil, d: 4.0}, T::Hash[Symbol, T.nilable(T.any(Integer, String, Float))]).compact,
  T::Hash[Symbol, T.any(Integer, String, Float)]
)

T.assert_type!(
  T.let({a: :one, b: nil, c: :three}, T::Hash[Symbol, T.nilable(Symbol)]).compact,
  T::Hash[Symbol, Symbol]
)

T.assert_type!(
  T.let({"a" => 1, "b" => nil, "c" => 3}, T::Hash[String, T.nilable(Integer)]).compact,
  T::Hash[String, Integer]
)

T.assert_type!(
  T.let({a: [1, 2], b: nil, c: {x: 1, y: nil}}, T::Hash[Symbol, T.nilable(T.any(T::Array[Integer], T::Hash[Symbol, T.nilable(Integer)]))]).compact,
  T::Hash[Symbol, T.any(T::Array[Integer], T::Hash[Symbol, T.nilable(Integer)])]
)

T.assert_type!(
  T.let({a: nil, b: nil}, T::Hash[Symbol, T.untyped]).compact,
  T::Hash[Symbol, T.untyped]
)

T.assert_type!(
  T.let({a: true, b: false, c: nil}, T::Hash[Symbol, T.nilable(T::Boolean)]).compact,
  T::Hash[Symbol, T::Boolean]
)

T.assert_type!(
  T.let({single_key: "value"}, T::Hash[Symbol, String]).compact,
  T::Hash[Symbol, String]
)

class MyHash < Hash
  extend T::Generic

  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)

  Another = type_member(:out)
end

T.assert_type!(
  MyHash.new.compact,
  T::Hash[T.anything, T.anything]
)
