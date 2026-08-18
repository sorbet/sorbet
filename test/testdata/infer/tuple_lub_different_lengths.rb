# typed: true

integers = if T.unsafe(nil)
  [1, 2]
else
  [1, 2, 3]
end
T.reveal_type(integers) # error: Revealed type: `T::Array[Integer]`

mixed = if T.unsafe(nil)
  [1, "two"]
else
  [3, 4, 5]
end
T.reveal_type(mixed) # error: Revealed type: `T::Array[T.any(Integer, String)]`
