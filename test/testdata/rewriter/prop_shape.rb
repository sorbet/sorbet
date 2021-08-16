# typed: true
extend T::Sig

class StructWithShapes < T::Struct
  prop :empty_shape, {}
  prop :symbol_key_shape, {foo: Integer}
  prop :string_key_shape, {'foo' => Integer}
  prop :multiple_keys, {foo: Integer, bar: String, baz: Symbol}
  prop :array_of_shape, T::Array[{foo: Integer}]
  prop :shape_with_combinators, {foo: T.nilable(Integer)}
  prop :combinator_with_shape, T.nilable({foo: Integer})
end

class StructWithBadShapes < T::Struct
  prop :bad_literal, {foo: 42}
end

x = StructWithShapes.new(
  empty_shape: {},
  symbol_key_shape: {foo: 0},
  string_key_shape: {'foo' => 0},
  multiple_keys: {foo: 0, bar: '', baz: :a_symbol},
  array_of_shape: [{foo: 0}],
  shape_with_combinators: {foo: nil}
)

T.reveal_type(x.empty_shape) # error: `{} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(x.symbol_key_shape) # error: `{foo: Integer} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(x.string_key_shape) # error: `{String("foo") => Integer} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(x.multiple_keys) # error: `{foo: Integer, bar: String, baz: Symbol} (shape of T::Hash[T.untyped, T.untyped])`

T.reveal_type(x.array_of_shape) # error: `T::Array[{foo: Integer}]`
T.reveal_type(x.shape_with_combinators) # error: `{foo: T.nilable(Integer)} (shape of T::Hash[T.untyped, T.untyped])`
T.reveal_type(x.combinator_with_shape) # error: `T.nilable({foo: Integer})`
