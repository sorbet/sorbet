# typed: true

arr = T.let(Array.new, T::Array[Integer])

init = T.let([], T::Array[Integer])

arr.each_with_object(init) do |k, arr|
  T.reveal_type(k) # error: Revealed type: `Integer`

  T.reveal_type(arr) # error: Revealed type: `T::Array[Integer]`
  arr << k
  arr << ''
  #      ^^ error: Expected `Integer` but found `String("")` for argument `arg0`
end

T.reveal_type(arr.each_with_object(T.let(0, Integer))) # error: Revealed type: `T::Enumerator[[Integer, Integer]]`
