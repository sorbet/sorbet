# typed: true

owners = T.let(1, T.untyped)
T.reveal_type(Array(owners)) # error: Revealed type: `T::Array[T.untyped]`

T.reveal_type(Array(10)) # error: Revealed type: `T::Array[T.untyped]`
