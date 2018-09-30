# typed: true

class Test
  extend T::Helpers

  sig {params(x: Integer, y: Integer).returns(Integer)}
  def foo(x: 0, y: x)
    x + y
  end

  sig {params(x: Integer, y: Integer).returns(Integer)}
  def bar(x: y, y: x) # error: Method `y` does not exist on `Test`
    x + y
  end

  sig {params(x: String, y: Integer).returns(String)}
  def qux(x: '', y: x) # error: Argument does not have asserted type `Integer`
    x + y.to_s
  end
end
