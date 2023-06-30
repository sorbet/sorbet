# typed: true
class TestUntyped
  extend T::Sig
  
  sig { returns(T.untyped) }
  def test2
    2
  end

  def local_call
    a = test2
  end

  def unsafe
    T.unsafe(1)
  end

  def method_call
    Method.new.call
  end
end
