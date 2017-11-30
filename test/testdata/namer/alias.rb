module A
  class C1; end
  C2 = C1

  standard_method({x: C2}, returns: NilClass)
  def f(x)
  end
end
