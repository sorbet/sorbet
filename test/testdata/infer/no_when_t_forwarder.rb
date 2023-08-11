# typed: true

case T.unsafe(nil)
when T::Array
  #  ^^^^^^^^ error: Use `Array` without any `T::` prefix to match on a stdlib generic type
when T::Hash
  #  ^^^^^^^ error: Use `Hash` without any `T::` prefix to match on a stdlib generic type
when T::Set
  #  ^^^^^^ error: Use `Set` without any `T::` prefix to match on a stdlib generic type
end

T::Array.===([])
#        ^^^ error: Use `Array` without any `T::` prefix to match on a stdlib generic type
