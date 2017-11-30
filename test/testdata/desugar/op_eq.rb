# @typed
class OpEq
  def b; end
  def b=(_); end
  def y; end
  def z; end

  def example(a)
    a &&= :a
    self.b &&= :b
    a[y, z] &&= 1


    a ||= :a
    self.b ||= :b
    a[y, z] ||= 1
  end
end
