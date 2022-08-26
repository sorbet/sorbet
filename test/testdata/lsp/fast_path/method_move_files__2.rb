# typed: true

class A
  extend T::Sig

  sig {returns(Integer)}
  def m
    0
  end
end
