# typed: true

class Animal; end
class Cat < Animal; end
class Persian < Cat; end

class A1
  extend T::Sig
  extend T::Generic

  T1 = type_member

  # should fail: T2 mentions another type member
  T2 = type_member(lower: T1)
     # ^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported

  # should fail: T3 mentions another type member
  T3 = type_member(lower: T2, upper: T1)
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported

  # should fail: T4 mentions another type member (this message could be better)
  T4 = type_member(fixed: T1)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expression does not have a fully-defined type

  # should fail: the bounds are invalid
  T5 = type_member(lower: Animal, upper: Persian)
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported

  # should fail: type member used as an argument to another type (this message
  # could be better)
  T6 = type_member(fixed: T::Array[T1])
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expression does not have a fully-defined type

  # should fail: type member used as an argument to another type
  T7 = type_member(lower: T::Array[T1])
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported

  # should fail: both bounds and a fixed type are specified
  T8 = type_member(fixed: Cat, lower: Persian, upper: Animal)
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Type member is defined with bounds and `:fixed`
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported

  # should pass: just an alias to a type member
  T9 = T1

  # should fail: still using a type member in the definition of another
  T10 = type_member(fixed: T9)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expression does not have

  # should fail: using a type alias in bounds
  T11 = type_member(upper: T9)
      # ^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported

  # should fail: multiple upper and lower bounds specified
  T12 = type_member(lower: Integer, lower: String, upper: BasicObject, upper: TrueClass)
      # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed`
end

module M
  extend T::Sig
  extend T::Generic

  X = type_member(lower: Persian, upper: Animal)
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported
end

class A2
  extend T::Sig
  extend T::Generic

  # This should allow instantiations of this variable as any of Animal, Cat, or
  # Persian.
  X = type_member(lower: Persian, upper: Animal)
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported
end

class Test
  extend T::Sig

  # should pass: the type given is valid in the context of the Animal/Persian
  # bounds
  sig {params(arg1: A2[Animal], arg2: A2[Cat], arg3: A2[Persian]).void}
  def test1(arg1, arg2, arg3); end

  # should fail: Integer is not within the Animal/Persian bounds.
  sig {params(arg1: A2[Integer], arg2: A2[BasicObject]).void}
  def test2(arg1, arg2); end
end
