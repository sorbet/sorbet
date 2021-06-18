# typed: true

T.reveal_type([1, nil].compact) # error: Revealed type: `T::Array[Integer]`

T.reveal_type([1, 2, 3].compact) # error: Revealed type: `T::Array[Integer]`

T.reveal_type([[1], nil].compact)  # error: Revealed type: `T::Array[[Integer(1)]]`

T.reveal_type(T::Hash[Symbol, T.nilable(Integer)].new.compact) # error: Revealed type: `T::Hash[Symbol, Integer]`

T.reveal_type(T::Hash[String, T.nilable(Integer)].new.compact) # error: Revealed type: `T::Hash[String, Integer]`

T.reveal_type({foo: 1, bar: 2}) # error: Revealed type: `{foo: Integer(1), bar: Integer(2)} (shape of T::Hash[T.untyped, T.untyped])`

T.reveal_type({foo: 1, bar: 2}.compact) # error: Revealed type: `T.untyped`

T.reveal_type({foo: 1, bar: nil}.compact) # error: Revealed type: `T.untyped`

T.reveal_type({foo: 1, bar: T.let(nil, T.nilable(Integer))}.compact) # error: Revealed type: `T.untyped`

def my_method(foo:, bar:); end

my_method({foo: 1, bar: nil}.compact)
