# typed: true

class Animal; end
class Cat < Animal; end

class Other; end

class Makes
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  Ty = type_member(:out) # error: Classes can only have invariant type members
end

class Needs
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  Ty = type_member(:in) # error: Classes can only have invariant type members
end

class Test
  extend T::Sig

  sig {params(m: Makes[Animal]).void}
  def self.test_makes(m); end

  sig {params(m: Needs[Cat]).void}
  def self.test_needs(m); end
end

# should pass: Makes::Ty is Animal
Test.test_makes(T.let(Makes.new, Makes[Animal]))

# should pass: Makes::Ty < Animal and Makes::Key is covariant
Test.test_makes(T.let(Makes.new, Makes[Cat]))

# should fail: Other isn't related to Animal
Test.test_makes(T.let(Makes.new, Makes[Other])) # error: Expected `Makes[Animal]` but found `Makes[Other]`

# should pass: Needs::Ty is Animal
Test.test_needs(T.let(Needs.new, Needs[Animal]))

# should pass: Animal < Makes::Ty and Needs::Key is contravariant
Test.test_needs(T.let(Needs.new, Needs[Cat]))

# should fail: Other isn't related to Animal
Test.test_needs(T.let(Needs.new, Needs[Other])) # error: Expected `Needs[Cat]` but found `Needs[Other]`
