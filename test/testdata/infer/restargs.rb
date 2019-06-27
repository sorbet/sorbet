# typed: true

class A
  extend T::Sig

  # Repeated argument is typed using element type
  sig {params(xs: Integer).void}
  def self.foo(*xs)
    # loads of repeated args become Array of element type
    T.reveal_type(xs) # error: Revealed type: `T::Array[Integer]`
  end
end

# Each repeated argument is checked as element type
A.foo(1, 2, '') # error: Expected `Integer` but found `String("")` for argument `xs`
A.foo(1, 2, 3)
