# @typed
class BasicError
  def basicError
    MissingConstant
  end
end

class ComplexError
  sig(arg: Integer).returns(NilClass)
  def foo(arg)
    arg
    raise arg # raise is defined by stdlib
  end

  def complexError
    foo("foo")
  end
end
