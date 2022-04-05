# typed: true

class MyClass
  def foo(x)
    x + THE_CONSTANT # error: Unable to resolve constant
      # ^ hover: This constant is not defined
  end
end
