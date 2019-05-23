# typed: true
class Foo2 < A # error: Unable to resolve constant `A`
  def branch
    1 + "stuff"
  end
end
