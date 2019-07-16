# typed: true

class Animal; end
class Cat < Animal; end
class Persian < Cat; end

class A1
  extend T::Sig
  extend T::Generic

  X = type_member(fixed: Cat, lower: Persian, upper: Animal)
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Type member is defined with bounds and `:fixed`
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported
end

class A2
  extend T::Sig
  extend T::Generic

  # This should allow instantiations of this variable as any of Animal, Cat, or
  # Persian.
  X = type_member(lower: Persian, upper: Animal)
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Only `:fixed` type members are supported
end

module M
  extend T::Sig
  extend T::Generic

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
