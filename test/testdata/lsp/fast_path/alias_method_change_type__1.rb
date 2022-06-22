# typed: true

class A
  extend T::Sig

  sig {returns(Integer)}
  def to_method; 0; end
end
