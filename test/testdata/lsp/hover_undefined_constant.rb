# typed: true

class MyClass
  def foo(x)
    x + THE_CONSTANT # error: Unable to resolve constant
      # ^ hover: T.untyped
  end
end
