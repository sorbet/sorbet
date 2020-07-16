# typed: strict

class BClass
  extend T::Sig

  sig {returns(A::AClass)}
  def get_a
    A::AClass.new()
  end
end
