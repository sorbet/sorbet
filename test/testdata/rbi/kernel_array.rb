# typed: true

owners = T.let(1, T.untyped)
T.reveal_type(Array(owners)) # error: Revealed type: `T::Array[T.untyped]`

T.reveal_type(Array(10)) # error: Revealed type: `T::Array[Integer(10)]`

T.reveal_type(Array([10])) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(Array(nil)) # error: Revealed type: `T::Array[T.untyped]`

x = T.let('', T.any(String, T::Array[String]))
T.reveal_type(Array(x)) # error: Revealed type: `T::Array[String]`

y = T.let('', String)
T.reveal_type(Array(y)) # error: Revealed type: `T::Array[String]`

T.reveal_type(Array('a'..'z')) # error: Revealed type: `T::Array[String]`

T.reveal_type(Array(1..10)) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(Array([1, 'a', 3])) # error: Revealed type: `T::Array[T.any(String, Integer)]`

maybe_x = T.let(nil, T.nilable(String))
res = Array(maybe_x)
T.reveal_type(res) # error: Revealed type: `T::Array[String]`
