# typed: true

class Animal; end
class Cat < Animal; end
class Serval < Cat; end

class A
  extend T::Generic
  T1 = type_member {{lower: Serval, upper: Animal}}
end

# should pass: Cat is within the bounds of T1
class B1 < A
  extend T::Generic
  T1 = type_member {{fixed: Cat}}
end

# should fail: String is not within the bounds
class B2 < A
  extend T::Generic
  T1 = type_member {{fixed: String}}
  #                         ^^^^^^ error: The `fixed` type bound `String` must be a supertype of the parent's `lower` type bound `Serval` for type_member `T1`
  #                         ^^^^^^ error: The `fixed` type bound `String` must be a subtype of the parent's `upper` type bound `Animal` for type_member `T1`
end

# should pass: the bounds are a refinement of the ones on A
class C1 < A
  extend T::Generic
  T1 = type_member {{lower: Serval, upper: Cat}}
end

# should fail: the bounds are wider than on A
class C2 < A
  extend T::Generic
  T1 = type_member {{lower: Serval, upper: Object}}
  #                                        ^^^^^^ error: The `upper` type bound `Object` must be a subtype of the parent's `upper` type bound `Animal` for type_member `T1`
end

# should fail: the implicit bounds of top and bottom are too wide for T1
class D1 < A
  T1 = type_member
     # ^^^^^^^^^^^ error: The `lower` type bound `T.noreturn` must be a supertype of the parent's `lower` type bound `Serval` for type_member `T1`
     # ^^^^^^^^^^^ error: The `upper` type bound `T.anything` must be a subtype of the parent's `upper` type bound `Animal` for type_member `T1`
end

