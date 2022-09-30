# typed: true

class Parent
  extend T::Generic
  TParent = type_member
end

class DifferentArityChild < Parent # error: Type `TParent` declared by parent `Parent` must be re-declared in `DifferentArityChild`
  TChild = type_member {{fixed: String}}
end

T.cast(1, DifferentArityChild[Integer, String, Symbol]) # error-with-dupes: Wrong number of type parameters for `DifferentArityChild`. Expected: `0`, got: `3`
