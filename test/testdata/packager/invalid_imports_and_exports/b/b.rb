# typed: strict

class BClass
  extend T::Sig 

  sig {returns(Integer)}
  def get_one
    A.one
  end
end

module BModule
  extend T::Sig

  sig {returns(A::AClass)}
  def get_a
    A::AClass.new
  end
end
