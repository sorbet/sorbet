# typed: strict

# This file is not valid Ruby since we reference A before
# definition. However, it would work with our autoloader, and also
# writing definitions out-of-order simulates the definitions being
# located in multiple files that are processed out of order by
# sorbet.
A::AB::BC

class HasError
  include D::DA::DOES_NOT_EXIST # error-with-dupes: Unable to resolve constant
end

class IsGood
  extend T::Sig

  sig {params(a: E::EC).returns(Integer)}
  def foo1(a)
    1
  end

  sig {params(a: E::EA).returns(Integer)}
  def foo2(a)
    1
  end
end

class TestInheritace1 < A::AB
end

class TestInheritace2 < E::EA # error: The super class `A` of `TestInheritace2` does not derive from `Class`
end

module A
  AB = B

  AV = T.let(1, Integer)
end

class B
  BC = C
end

class C < B
end

module F
  include E

  # Assert that this resolves to the right thing. Previously we would
  # recognize that we were hitting an ordering problem, and stub to
  # `untyped`.
  T.assert_type!(EA::AV, Integer)
end

module E
  include D
  EA = DA
  EC = B::BC
end

module D
  DA = A
end
