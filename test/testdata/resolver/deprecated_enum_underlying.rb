# typed: true

class A

  extend T::Sig

  # Verifies that the underlying type for the literals is used when constructing
  # the union that approximates the `T.deprecated_enum` type.
  sig {params(x: T.deprecated_enum(["a", 1, :foo, true, false])).void}
  def foo(x)
    T.reveal_type(x) # error: Revealed type: `T.any(String, Integer, Symbol, TrueClass, FalseClass)`
  end

  sig { params(x: T.deprecated_enum([:foo])).void }
  def symbol_foo(x)
    T.reveal_type(x) # error: `Symbol`
  end

  sig { params(x: T.deprecated_enum([0])).void }
  def integer_0(x)
    T.reveal_type(x) # error: `Integer`
  end
end
