# typed: false

class A
  def self.foo
    X = 10
#   ^ parser-error: dynamic constant assignment
  end
end
