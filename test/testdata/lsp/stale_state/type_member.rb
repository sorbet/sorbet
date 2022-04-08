# typed: true

class A
  extend T::Generic

  Elem = type_member
  # ^ hover: A::Elem

  # ___
end
