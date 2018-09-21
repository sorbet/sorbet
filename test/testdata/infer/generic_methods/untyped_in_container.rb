# typed: true
extend T::Generic

type_parameters(:T)
  .sig(arr: T::Array[T.type_parameter(:T)])
  .returns(T.type_parameter(:T))
def lub_elems(*arr)
  return arr.fetch(0).fetch(0)
end

T.reveal_type(
  lub_elems(T.unsafe(nil))) # error: type: `T.untyped`

# `T.untyped`, for now, "wins" if LUBed with other types
T.reveal_type(
  lub_elems(T.unsafe(nil), T::Array[Integer].new)) # error: type: `T.untyped`


T.reveal_type(Hash[T::Array[T.untyped].new]) # error: Revealed type: `T::Hash[T.untyped, T.untyped]`
