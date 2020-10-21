# typed: true
class Test
  extend T::Sig

  sig {params(x: String).returns(String)}
  def foo(x = nil) # error: Argument does not have asserted type `String`
    x
  # ^ error: Returning value that does not conform to method result type
  end

  sig {params(y: String).returns(String)}
  def bar(y: nil) # error: Argument does not have asserted type `String`
    y
  # ^ error: Returning value that does not conform to method result type
  end

  sig {params(z: String).returns(String)}
  def qux(z = '')
    z
  end

  sig {params(w: String).returns(String)}
  def baz(w: '')
    w
  end
end
