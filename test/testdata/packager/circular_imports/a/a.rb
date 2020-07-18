# typed: strict

class AClass
  extend T::Sig

  sig {returns(B::BClass)}
  def get_b
    B::BClass.new()
  end
end
