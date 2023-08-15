# typed: true

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig do
    abstract
      .params(bar: T::Array[Integer], baz: Symbol, qux: String, quux: T.nilable(Float), corge: T.nilable(T::Boolean))
      .returns(T.nilable(T::Hash[Symbol, T.untyped]))
  end
  def foo(bar, baz, qux, quux = nil, corge: true); end
end

  class Child < Parent
# ^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method
  end
