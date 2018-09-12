# typed: true
class A
  extend T::Helpers

  sig
  .returns(Integer)
  def foo # error: does not conform to method result type
  end
end

class Bar < A
  extend T::Helpers

  sig(
    arg: Integer
  )
  .returns(Integer)
  def baz(arg)
    foo
  end
end
