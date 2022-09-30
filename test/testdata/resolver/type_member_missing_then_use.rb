# typed: true

class Child < Parent; end # error: Type `X` declared by parent `Parent` must be re-declared in `Child`
class Parent < Grandparent; end # error: Type `X` declared by parent `Grandparent` must be re-declared in `Parent`
class Grandparent
  extend T::Generic
  X = type_member
end

class ChildMod < ParentMod; end # error: Type `X` declared by parent `ParentMod` must be re-declared in `ChildMod`
module Mod
  extend T::Generic
  X = type_member
end
class ParentMod # error: Type `X` declared by parent `Mod` must be re-declared in `ParentMod`
  include Mod
end
