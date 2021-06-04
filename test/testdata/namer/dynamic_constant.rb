# typed: false

class A
  def self.foo
    X = 10
#   ^ error: dynamic constant assignment
  end

  [].each do
    Y = 30
#   ^ error: Dynamic constant assignment
  end

  foo { ZZZ = 40 }
#       ^^^ error: Dynamic constant assignment
end
