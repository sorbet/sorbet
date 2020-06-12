# typed: strict

REFERENCE = ASecondClass

class ASecondClass

end

class AClass
  extend T::Sig 

  sig {returns(AClass)}
  def get_a
    B.get_a
  end
end


module AModule
  extend T::Sig

  sig {returns(Integer)}
  def one
    1
  end
end
