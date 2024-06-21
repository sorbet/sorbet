# typed: true

class A
  extend T::Sig

  sig {params(x: Integer).void}
  sig {params(x: Integer, y: String).void}
  def self.example(x, y='') # error: against an overloaded signature
  end

  sig {params(x: Integer).void}
  sig {params(x: Integer, y: String).void}
  sig {params(x: Integer, y: String, z: Float).void}
  def self.example(x, y='', z=0.0) # error: Method `A.example` redefined without matching argument count. Expected: `2`, got: `3`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: against an overloaded signature
  end
end

A.example # error: Not enough arguments
A.example(0)
A.example(0, '')
A.example(0, '', 0.0)
A.example1 # error: Method `example1` does not exist
A.example1(0) # error: Method `example1` does not exist
A.example1(0, '') # error: Method `example1` does not exist
A.example1(0, '', 0.0) # error: Method `example1` does not exist
