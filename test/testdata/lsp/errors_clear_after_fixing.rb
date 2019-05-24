# typed: true
class Foo1
  def branch
    1 + "stuff" # error: does not match `Integer`
  end
end
