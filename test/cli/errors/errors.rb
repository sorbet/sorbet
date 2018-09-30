# typed: true
class BasicError
  def basicError
    MissingConstant
  end
end

class ComplexError
  extend T::Helpers

  sig {params(arg: Integer).returns(NilClass)}
  def foo(arg)
    arg
    raise arg # raise is defined by stdlib
  end

  def complexError
    foo("foo")
  end
end

class ErrorLines
  extend T::Helpers

  def main(foo)
    a = case foo
        when 1
          nil
        when 2
          2
        end
    bar(a)
  end

  sig {params(a: Integer).returns(Integer)}
  def bar(a)
    a
  end
end
