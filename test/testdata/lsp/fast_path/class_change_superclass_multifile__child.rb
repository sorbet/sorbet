# typed: true

class Foo2 < A
  def branch
    1 + "stuff" # error: Expected `Integer`
  end
end
