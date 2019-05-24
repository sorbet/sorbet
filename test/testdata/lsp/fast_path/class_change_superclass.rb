# typed: true

class A
end

class B
end

class Foo2 < A
  def branch
    1 + "stuff" # error: does not match `Integer`
  end
end
