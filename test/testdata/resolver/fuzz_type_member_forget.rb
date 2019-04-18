# typed: true
# disable-fast-path: true
class Parent
  extend T::Generic
  TParent = type_member
end

class DifferentArityChild < Parent # error: Type `TParent` declared by parent `Parent` must be declared again
  TChild = type_member(fixed: String)
end

T.cast(1, DifferentArityChild[Integer, String, Symbol]) # error: Wrong number of type parameters for `DifferentArityChild`. Expected: `0`, got: `3`
