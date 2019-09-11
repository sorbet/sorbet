# typed: true

class Animal; end
class Cat < Animal; end

class Other; end

module Makes
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  Ty = type_member(:out)

  sig {returns(Makes[T.untyped])}
  def self.make;
    T.unsafe(nil)
  end
end

module Needs
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  Ty = type_member(:in)

  sig {returns(Needs[T.untyped])}
  def self.make;
    T.unsafe(nil)
  end
end

class Test
  extend T::Sig

  sig {params(m: Makes[Animal]).void}
  def self.test_makes(m); end

  sig {params(m: Needs[Cat]).void}
  def self.test_needs(m); end
end

# should pass: Makes::Ty is Animal
Test.test_makes(T.let(Makes.make, Makes[Animal]))

# should pass: Makes::Ty < Animal and Makes::Key is covariant
Test.test_makes(T.let(Makes.make, Makes[Cat]))

# should fail: Other isn't related to Animal
Test.test_makes(T.let(Makes.make, Makes[Other])) # error: Expected `Makes[Animal]` but found `Makes[Other]`

# should pass: Needs::Ty is Animal
Test.test_needs(T.let(Needs.make, Needs[Animal]))

# should pass: Animal < Makes::Ty and Needs::Key is contravariant
Test.test_needs(T.let(Needs.make, Needs[Cat]))

# should fail: Other isn't related to Animal
Test.test_needs(T.let(Needs.make, Needs[Other])) # error: Expected `Needs[Cat]` but found `Needs[Other]`
