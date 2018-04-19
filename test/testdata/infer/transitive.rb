# typed: strict
class A
  sig
  .returns(Integer)
  def foo # error: does not conform to method result type
  end
end

class Bar < A
  sig(
    arg: Integer
  )
  .returns(Integer)
  def baz(arg)
    foo
  end
end
