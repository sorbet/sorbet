# typed: false

class A
  def self.foo
    X = 10
#   ^ error: dynamic constant assignment
  end
end
