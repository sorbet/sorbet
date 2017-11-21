class A
  standard_method(
    {
    },
    returns: Integer
  )
  def foo
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
