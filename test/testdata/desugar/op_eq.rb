class OpEq
  def example(a)
    a &&= :a
    self.b &&= :b
    x[y, z] &&= 1


    a ||= :a
    self.b ||= :b
    x[y, z] ||= 1
  end
end
