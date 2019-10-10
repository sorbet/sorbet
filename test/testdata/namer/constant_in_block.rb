# typed: true
class A

  def self.foo
    foo do
    C = 1
#   ^ error: dynamic constant assignment
    C # error: Unable to resolve constant
    end
  end

  [].each do
    B = 1
    B
  end
end
