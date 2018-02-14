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

class TestConfigatron
  def configatron_test
      T.assert_type!(configatron.test_bool, T.any(TrueClass, FalseClass))
      T.assert_type!(configatron.test_int, Integer)
      T.assert_type!(configatron.test_float, Float)
      T.assert_type!(configatron.test_string, String)
  end
end
