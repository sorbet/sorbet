# typed: true

class Animal; end
class Cat < Animal; end
class Persian < Cat; end

class A1
  extend T::Sig
  extend T::Generic

  T1 = type_member

  # should fail: T2 mentions another type member
  T2 = type_member {{lower: T1}}
                          # ^^ error: is not allowed

  # should fail: T3 mentions another type member
  T3 = type_member {{lower: T2, upper: T1}}
                          # ^^ error: is not allowed in this context
                                     # ^^ error: is not allowed in this context

  # should fail: T4 mentions another type member (this message could be better)
  T4 = type_member {{fixed: T1}}
                          # ^^ error: is not allowed in this context

  # should fail: the bounds are invalid
  T5 = type_member {{lower: Animal, upper: Persian}}
     # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: The `lower` type bound `Animal` is not a subtype of the `upper` type bound `Persian` for `A1::T5`

  # should fail: type member used as an argument to another type
  T6 = type_member {{fixed: T::Array[T1]}}
                                   # ^^ error: is not allowed in this context

  # should fail: type member used as an argument to another type
  T7 = type_member {{lower: T::Array[T1]}}
                                   # ^^ error: is not allowed in this context

  # should fail: both bounds and a fixed type are specified
  T8 = type_member {{fixed: Cat, lower: Persian, upper: Animal}}
  #                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Type member is defined with bounds and `fixed`

  # should pass: just an alias to a type member
  T9 = T1

  # should fail: still using a type member in the definition of another
  T10 = type_member {{fixed: T9}}
                           # ^^ error: is not allowed in this context

  # should fail: using a type alias in bounds
  T11 = type_member {{upper: T9}}
                           # ^^ error: is not allowed in this context

  # should fail: multiple upper and lower bounds specified
  T12 = type_member {{lower: Integer, lower: String, upper: BasicObject, upper: TrueClass}}
      # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: The `lower` type bound `String` is not a subtype of the `upper` type bound `TrueClass` for `A1::T12`
                                    # ^^^^^ error: Hash key `lower` is duplicated
                                                                       # ^^^^^ error: Hash key `upper` is duplicated

  # should fail: multiple fixed values provided
  T13 = type_member {{fixed: Integer, fixed: String}}
                                    # ^^^^^ error: Hash key `fixed` is duplicated

  T14 = type_member { {} }
  #                   ^^ error: Type member bounds must use either `fixed` or `lower`/`upper`
end

module M
  extend T::Sig
  extend T::Generic

  X = type_member {{lower: Persian, upper: Animal}}
end

class A2
  extend T::Sig
  extend T::Generic

  # This should allow instantiations of this variable as any of Animal, Cat, or
  # Persian.
  X = type_member {{lower: Persian, upper: Animal}}
end

class Test
  extend T::Sig

  # should pass: the type given is valid in the context of the Animal/Persian
  # bounds
  sig {params(arg1: A2[Animal], arg2: A2[Cat], arg3: A2[Persian]).void}
  def test1(arg1, arg2, arg3); end

  # There are duplicate errors here because both resolver and infer report the same error. This is not intentional.
  # should fail: Integer is not within the Animal/Persian bounds.
  sig {params(arg1: A2[Integer], arg2: A2[BasicObject]).void}
  #                    ^^^^^^^ error-with-dupes: `Integer` is not a subtype of upper bound of type member `::A2::X`
  #                    ^^^^^^^ error-with-dupes: `Integer` is not a supertype of lower bound of type member `::A2::X`
  #                                       ^^^^^^^^^^^ error-with-dupes: `BasicObject` is not a subtype of upper bound of type member `::A2::X`
  def test2(arg1, arg2); end
end
