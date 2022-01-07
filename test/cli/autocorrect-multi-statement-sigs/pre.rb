# typed: true

class Test
  extend T::Sig

  sig do
    overridable
    params(a: String)
    returns(Integer)
  end
  def foo(a)
    10
  end

end
