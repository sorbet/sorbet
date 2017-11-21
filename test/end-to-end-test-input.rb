class BasicError
  def basicError
    MissingConstant
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
