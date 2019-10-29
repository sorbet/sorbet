# typed: true
class Test
  extend T::Sig

  sig {params(x: Integer, y: Integer).returns(Integer)}
  def foo(x: 0, y: x)
    x + y
  end

  sig {params(x: Integer, y: Integer).returns(Integer)}
  def bar(x: y, y: x) # error: Method `y` does not exist on `Test`
    x + y
  end

  sig {params(x: String, y: Integer).returns(String)}
  def qux(x: '', y: x) # error: Returning value that does not conform to method result type
    x + y.to_s
  end
end
