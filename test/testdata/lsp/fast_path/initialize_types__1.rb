# typed: true
# Spacer so error assertions are on same line

class A
  extend T::Sig

  sig {params(x: Integer).void}
  def initialize(x)
    T.reveal_type(x) # error: `Integer`
  end
end

A.new(0)
A.new('') # error: Expected `Integer` but found `String("")`
