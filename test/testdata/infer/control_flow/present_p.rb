# typed: strict

class NilClass
  extend T::Helpers

  sig.returns(FalseClass)
  def present?
    false
  end
end

class Object
  def present?
    !self
  end
end

class A
  extend T::Helpers

  sig(s: T.nilable(String)).returns(NilClass)
  def test_return(s)
    if s.present?
      s.length
      nil
    end
    nil
  end

  sig(s: String).returns(NilClass)
  def test_unchanged(s)
    if s.present?
      s.length
    else
      s.length
    end
    nil
  end

  def unreachable()
    if nil.present?
      "foo" # error: This code is unreachable
    end
  end
end
