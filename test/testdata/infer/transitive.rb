# typed: true
class A
  extend T::Helpers

  sig do
    returns(Integer)
  end
  def foo # error: does not conform to method result type
  end
end

class Bar < A
  extend T::Helpers

  sig do
    params(
      arg: Integer
    )
    .returns(Integer)
  end
  def baz(arg)
    foo
  end
end
