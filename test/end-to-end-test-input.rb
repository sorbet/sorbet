class BasicError
  standard_method(
    {
      arg: Integer
    },
    returns: String
  )
  def basicError(arg)
    arg
  end
end

class ComplexError
  standard_method(
    {
      arg: Integer
    },
    returns: Integer
  )
  def foo(arg)
    arg
  end

  def complexError
    foo("foo")
  end
end

