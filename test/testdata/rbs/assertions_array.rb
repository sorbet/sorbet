# typed: strict
# enable-experimental-rbs-comments: true

arr1 = [
  1, #: as String
  2, #: as String
  [
    3 #: as String
  ], #: as Array[Integer]
]
T.reveal_type(arr1) # error: Revealed type: `[String, String, T::Array[Integer]] (3-tuple)`

[
  1,
  2,
] #: Array[Integer]

arr2 = [] #: Array[Integer]
T.reveal_type(arr2) # error: Revealed type: `T::Array[Integer]`

arr3 = [
  *arr2,
] #: Array[Integer]
T.reveal_type(arr3) # error: Revealed type: `T::Array[Integer]`

arr4 = [
  *arr2, #: as Array[String]
]
T.reveal_type(arr4) # error: Revealed type: `T::Array[String]`

arr5 = [
  **{a: 1},
] #: Array[T::Hash[Symbol, Integer]]
T.reveal_type(arr5) # error: Revealed type: `T::Array[T::Hash[Symbol, Integer]]`

arr_nil = {} #: Hash[Symbol, Integer]?
arr6 = [
  **arr_nil, #: as !nil
]
T.reveal_type(arr6) # error: Revealed type: `[T::Hash[Symbol, Integer]] (1-tuple)`
