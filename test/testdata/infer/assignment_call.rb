# typed: true
class A
  extend T::Sig

  sig { params(value: Integer).returns(NilClass) }
  def normal=(value)
  end

  sig { params(key: Symbol, value: Integer).returns(NilClass) }
  def []=(key, value)
  end

  sig { params(other: A).returns(Symbol) }
  def <=(other)
    :hello
  end

  sig { params(other: A).returns(String) }
  def !=(other)
    "!="
  end

  sig { params(other: A).returns(Integer) }
  def >=(other)
    1
  end

  sig { params(other: A).returns(Float) }
  def ===(other)
    0.1
  end
end

T.assert_type!(A.new.normal = 400, Integer)
T.assert_type!(A.new[:hows_your_day_going] = 400, Integer)
T.assert_type!(A.new <= A.new, Symbol)
T.assert_type!(A.new != A.new, String)
T.assert_type!(A.new >= A.new, Integer)
T.assert_type!(A.new === A.new, Float)
