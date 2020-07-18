# typed: strict

class SubpackageClass
  extend T::Sig

  sig {returns(Integer)}
  def self.calls_method
    # Tests that the import from a root package works.
    MainPackage::ClassWithMethod.method
  end
end
