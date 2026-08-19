# typed: true

T.reveal_type(
  if T.unsafe(nil)
    [1, 2]
  else
    [1, 2, 3]
  end
) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(
  if T.unsafe(nil)
    []
  else
    [1, 2, 3]
  end
) # error: Revealed type: `T::Array[Integer]`

T.reveal_type(
  if T.unsafe(nil)
    [1, "two"]
  else
    [1, 2, 3]
  end
) # error: Revealed type: `T::Array[T.untyped]`
