# @typed
class A
  standard_method(
    {
    },
    returns: Integer
  )
  def foo # error: does not conform to method result type
  end
end

class Bar < A
  standard_method(
    {
      arg: Integer
    },
    returns: Integer
  )
  def baz(arg)
    foo
  end
end
