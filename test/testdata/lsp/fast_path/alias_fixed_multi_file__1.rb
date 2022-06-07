# typed: true

class A
  extend T::Sig

  sig {returns(Integer)}
  def to; 0; end
end
