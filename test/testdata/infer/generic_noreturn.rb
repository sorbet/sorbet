# typed: strict
class MyGeneric
  extend T::Generic
  Elem = type_member
end

x = MyGeneric[T.noreturn].new
T.reveal_type(x) # error: Revealed type: `MyGeneric[T.noreturn]`
