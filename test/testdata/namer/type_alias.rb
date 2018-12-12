# typed: true
module A
  extend T::Sig

  class C1; end
  C2 = C1

  sig {params(x: C2).returns(NilClass)}
  def f(x)
  end
end
