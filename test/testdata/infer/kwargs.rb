# typed: true

class A
  extend T::Sig

  # Repeated argument is typed using value type (key is always Symbol)
  sig {params(kwargs: Integer).void}
  def self.foo(**kwargs)
    # loads of kwargs become Hash of value type
    T.reveal_type(kwargs) # error: Revealed type: `T::Hash[Symbol, Integer]`
  end
end

# Each repeated argument is checked as element type
A.foo(x: 1, y: 2, z: '') # error: `String("")` doesn't match `Integer` for argument `kwargs`
A.foo(x: 1, y: 2, z: 3)
