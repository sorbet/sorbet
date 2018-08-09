# typed: strict

class TestSelf
  extend T::Helpers

  sig(s: self)
  .returns(self)
  def good1(s)
    self
  end

  sig()
    .returns(self)
  def pass()
    good1(self)
  end
end


class TestSelfGeneric
  extend T::Generic

  Elem = type_member
  sig(s: self)
  .returns(self)
  def good1(s)
    self
  end

  sig()
    .returns(self)
  def pass()
    good1(self)
  end
end
