# typed: strict

foo = T::Array[Integer].new
foo.inject(nil) {|sum, x| sum ? sum + x : x}
#                               ^^^^^^^ error: This code is unreachable
foo.inject(T.unsafe(nil)) do |sum, x|
  T.reveal_type(sum) # error: `T.untyped`
  T.reveal_type(x) # error: `Integer`
  sum ? sum + x : x
end
foo.inject(T.let(nil, T.nilable(Integer))) do |sum, x|
  T.reveal_type(sum) # error: `T.nilable(Integer)`
  T.reveal_type(x) # error: `Integer`
  sum ? sum + x : x
end
