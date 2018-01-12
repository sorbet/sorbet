# @typed
module A
  class C1; end
  C2 = C1

  sig(x: C2).returns(NilClass)
  def f(x)
  end
end
