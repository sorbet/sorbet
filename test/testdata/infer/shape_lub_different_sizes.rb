# typed: true

homogeneous = if T.unsafe(nil)
  { a: 1 }
else
  { b: 2, c: 3 }
end
T.reveal_type(homogeneous) # error: Revealed type: `T::Hash[Symbol, Integer]`

empty = if T.unsafe(nil)
  {}
else
  { a: 1, b: 2 }
end
T.reveal_type(empty) # error: Revealed type: `T::Hash[Symbol, Integer]`

heterogeneous_values = if T.unsafe(nil)
  { a: 1 }
else
  { b: 2, c: "three" }
end
T.reveal_type(heterogeneous_values) # error: Revealed type: `T::Hash[T.untyped, T.untyped]`

heterogeneous_keys = if T.unsafe(nil)
  { a: 1 }
else
  { b: 2, "c" => 3 }
end
T.reveal_type(heterogeneous_keys) # error: Revealed type: `T::Hash[T.untyped, T.untyped]`

different_value_types = if T.unsafe(nil)
  { a: 1 }
else
  { b: "two", c: "three" }
end
T.reveal_type(different_value_types) # error: Revealed type: `T::Hash[T.untyped, T.untyped]`

different_key_types = if T.unsafe(nil)
  { a: 1 }
else
  { "b" => 2, "c" => 3 }
end
T.reveal_type(different_key_types) # error: Revealed type: `T::Hash[T.untyped, T.untyped]`
