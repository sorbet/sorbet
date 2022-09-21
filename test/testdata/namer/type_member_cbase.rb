# typed: strict

class A
  extend T::Generic

  ::X = type_member
  #     ^^^^^^^^^^^ error: Expected `Integer` but found `T::Types::TypeMember` for field
  Y = type_member
end

class B
  ::X = T.let(1, Integer)
end
