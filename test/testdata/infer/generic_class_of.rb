# typed: strict
class MyGeneric
  extend T::Generic
  Elem = type_member
end

class A; end

x = MyGeneric[T.class_of(A)].new
T.reveal_type(x) # error: Revealed type: `MyGeneric[T.class_of(A)]`

T.reveal_type(T::Array[T.class_of(Integer)].new) # error: Revealed type: `T::Array[T.class_of(Integer)]`
