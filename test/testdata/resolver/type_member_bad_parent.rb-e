# typed: true

class Parent
  Elem = 3
end

class Child < Parent
  extend T::Generic
  Elem = type_member # error: `Child::Elem` is a type member but `Parent::Elem` is not a type member
end
