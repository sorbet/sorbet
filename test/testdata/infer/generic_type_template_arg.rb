# typed: true

class A
  extend T::Generic
  X = type_template
end

T.reveal_type(T::Array[A].new) # error: Revealed type: `T::Array[A]`
T.reveal_type(T::Array[Integer].new) # error: Revealed type: `T::Array[Integer]`
