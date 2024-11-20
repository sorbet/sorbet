# typed: true

arr0 = []
a, *b = arr0
T.reveal_type(a) # error: Revealed type: `NilClass`
T.reveal_type(b) # error: Revealed type: `T::Array[NilClass]`

b = *arr0
arr1 = [1, 2, 3]
a, *b = arr1
T.reveal_type(a) # error: Revealed type: `Integer(1)`
T.reveal_type(b) # error: Revealed type: `T::Array[Integer]`

arr2 = [1, :foo, "foo"]
a, *b = arr2
T.reveal_type(a) # error: Revealed type: `Integer(1)`
T.reveal_type(b) # error: Revealed type: `T::Array[T.any(Integer, Symbol, String)]`

arr3 = T.let([], T::Array[T.untyped])
a, *b = arr3
T.reveal_type(a) # error: Revealed type: `T.untyped`
T.reveal_type(b) # error: Revealed type: `T::Array[T.untyped]`

arr4 = T.unsafe([])
a, *b = arr4
T.reveal_type(a) # error: Revealed type: `T.untyped`
T.reveal_type(b) # error: Revealed type: `T.untyped`

arr5 = [1, 2, 3]
a, *b, c = arr5
T.reveal_type(a) # error: Revealed type: `Integer(1)`
T.reveal_type(b) # error: Revealed type: `T::Array[Integer]`
T.reveal_type(c) # error: Revealed type: `Integer(3)`

arr6 = [1, 2, 3]
a, b, *c = arr6
T.reveal_type(a) # error: Revealed type: `Integer(1)`
T.reveal_type(b) # error: Revealed type: `Integer(2)`
T.reveal_type(c) # error: Revealed type: `T::Array[Integer]`

arr7 = [1, 2, 3]
a, *b = *arr7
T.reveal_type(a) # error: Revealed type: `Integer(1)`
T.reveal_type(b) # error: Revealed type: `T::Array[Integer]`

arr8 = []
a, *b = *T.unsafe(arr8)
T.reveal_type(a) # error: Revealed type: `T.untyped`
T.reveal_type(b) # error: Revealed type: `T.untyped`

arr9 = T.let(nil, T.nilable(T::Array[Integer]))
a, *b = arr9
T.reveal_type(a) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type(b) # error: Revealed type: `T::Array[T.nilable(Integer)]`
