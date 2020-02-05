# typed: strict

class MyEnum < T::Enum
  extend T::Sig

  sig {params(x: String, y: String).void}
  def initialize(x, y); end

  enums do
    X = new # error: Not enough arguments
    Y = new('a', 'b')
    Z = new('a', 0) # error: Expected `String` but found `Integer(0)`
  end
end
