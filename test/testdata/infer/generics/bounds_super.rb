# typed: true
# disable-fast-path: true

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
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: parent lower bound `Serval` is not a subtype of lower bound `String`
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: upper bound `String` is not a subtype of parent upper bound `Animal`
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
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: upper bound `Object` is not a subtype of parent upper bound `Animal`
end

# should fail: the implicit bounds of top and bottom are too wide for T1
class D1 < A
  T1 = type_member
     # ^^^^^^^^^^^ error: parent lower bound `Serval` is not a subtype of lower bound `T.noreturn`
     # ^^^^^^^^^^^ error: upper bound `<top>` is not a subtype of parent upper bound `Animal`
end

