# typed: strict

class Parent
  extend T::Sig
  extend T::Generic

  MyElem = type_member
end

class Child < Parent # error: Type `MyElem` declared by parent `Parent` must be re-declared in `Child`
end
