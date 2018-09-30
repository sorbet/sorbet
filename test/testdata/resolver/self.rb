# typed: true

class TestSelf
  extend T::Helpers

  sig do
    params(s: self)
    .returns(self)
  end
  def good1(s)
    self
  end

  sig do
    params()
      .returns(self)
  end
  def pass()
    good1(self)
  end
end


class TestSelfGeneric
  extend T::Generic

  Elem = type_member
  sig do
    params(s: self)
    .returns(self)
  end
  def good1(s)
    self
  end

  sig do
    params()
      .returns(self)
  end
  def pass()
    good1(self)
  end
end
