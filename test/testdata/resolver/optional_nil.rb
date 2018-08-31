# typed: true

class Test
  extend T::Helpers

  sig(x: String).returns(String)
  def foo(x = nil) # error: Argument does not have asserted type `String`
    x
  end

  sig(y: String).returns(String)
  def bar(y: nil) # error: Argument does not have asserted type `String`
    y
  end

  sig(z: String).returns(String)
  def qux(z = '')
    z
  end

  sig(w: String).returns(String)
  def baz(w: '')
    w
  end
end
