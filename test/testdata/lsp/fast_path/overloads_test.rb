# typed: true

class A
  extend T::Sig

  sig {params(x: Integer).void}
  sig {params(x: Integer, y: String).void}
  def self.example(x, y='') # error: against an overloaded signature
  end
end

A.example(0)
A.example(0, '')
A.example # error: Not enough arguments
A.example1(0) # error: Method `example1` does not exist
A.example1(0, '') # error: Method `example1` does not exist
A.example1 # error: Method `example1` does not exist
