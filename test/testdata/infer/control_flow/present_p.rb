# typed: true

class NilClass
  extend T::Sig

  sig {returns(FalseClass)}
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
  extend T::Sig

  sig {params(s: T.nilable(String)).returns(NilClass)}
  def test_return(s)
    if s.present?
      s.length
      nil
    end
    nil
  end

  sig {params(s: String).returns(NilClass)}
  def test_unchanged(s)
    if s.present?
      s.length
    else
      s.length
    end
    nil
  end

  def unreachable_nil()
    if nil.present?
      "foo" # error: This code is unreachable
    end
  end

  def unreachable_false()
    if false.present?
      "foo" # error: This code is unreachable
    end
  end

  def unreachable_true()
    if true.present?
      "bar"
    else
      "foo" # error: This code is unreachable
    end
  end
end
