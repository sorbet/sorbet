# typed: strict
class Parent
  extend T::Generic

  Elem = type_member
  Elem = type_member # error: Duplicate type member
end

class Child < Parent
  Elem = type_member
end
