# typed: true

class A
  def foo
    x # error: Method `x` does not exist on `A`
  end
end
