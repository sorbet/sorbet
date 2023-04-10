# typed: true
# disable-fast-path: true
class Parent
  extend T::Generic
  TParent = type_member
end

class DifferentArityChild < Parent # error: Type `TParent` declared by parent `Parent` must be re-declared in `DifferentArityChild`
  TChild = type_member {{fixed: String}}
end

T.cast(1, DifferentArityChild[Integer, String, Symbol]) # error-with-dupes: All type parameters for `DifferentArityChild` have already been fixed
