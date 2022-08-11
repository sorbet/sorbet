# typed: strict

class A
  extend T::Sig

  sig {returns(Integer)}
  def Foo; 0; end
end
