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
A.foo(x: 1, y: 2, z: '') # error: Expected `Integer` but found `String("")` for argument `kwargs`
A.foo(x: 1, y: 2, z: 3)


class B
  extend T::Sig

  sig {params(y: T::Hash[String,String], z: String).void}
  def self.foo(y={}, z:)
  end
end

# There is some tricky logic in dispatchCallSymbol to handle the case where a final positional hash argument is used for
# keyword args. For the following case, the hash should be used for the defaulted positional arg, /not/ the implicit
# keyword args hash.
B.foo({ "a" => "foo" }) # error: Missing required keyword argument `z`
