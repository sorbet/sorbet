# typed: strict

class ClassWithMethod
  extend T::Sig

  sig {returns(Integer)}
  def self.method
    10
  end
end
