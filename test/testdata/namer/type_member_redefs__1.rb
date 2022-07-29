# typed: true

class A
  extend T::Generic
  R = type_member
  R = type_member # error: Duplicate type member `R`
end

class B
  extend T::Generic
  R = type_member # error: Expected `Integer` but found `T::Types::TypeMember` for field
  R = 1 # error: Redefining constant `R`
end

class C
  extend T::Generic
  R = 1
  R = type_member # error: Redefining constant `C::R`
end

class D
  extend T::Generic
  R = type_member # error: Duplicate type member `R`
end
