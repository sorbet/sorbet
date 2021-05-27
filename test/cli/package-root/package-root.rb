# typed: strict

class MainPackage::ClassWithMethod
  extend T::Sig

  sig {returns(Integer)}
  def self.method
    10
  end
end
