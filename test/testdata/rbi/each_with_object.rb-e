# typed: true

arr = T.let(Array.new, T::Array[Integer])

init = T.let([], T::Array[Integer])

arr.each_with_object(T.let([], T::Array[Integer])) do |k, arr|
  T.reveal_type(k) # error: Revealed type: `Integer`

  # TODO: this should be `Integer`, but because of #38 is set to `T.untyped`
  T.reveal_type(arr) # error: Revealed type: `T.untyped`
end

T.reveal_type(arr.each_with_object(T.let(0, Integer))) # error: Revealed type: `T::Enumerator[[Integer, Integer]]`
