# typed: true

class Child < Parent; end # error: Type `X` declared by parent `Parent` should be declared again
class Parent < Grandparent; end # error: Type `X` declared by parent `Grandparent` should be declared again
class Grandparent
  include T::Generic
  X = type_member
end

class ChildMod < ParentMod; end # error: Type `X` declared by parent `ParentMod` should be declared again
module Mod
  include T::Generic
  X = type_member
end
class ParentMod # error: Type `X` declared by parent `Mod` should be declared again
  include Mod
end
