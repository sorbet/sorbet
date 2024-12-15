# typed: true

# One-line format
class MyAbstractClass
  extend T::Sig, T::Helpers
  abstract!

  sig {abstract.type_parameters(:U).params(x: T.type_parameter(:U)).returns(T.type_parameter(:U))}
  def example(x); end
end

class MyClass < MyAbstractClass
# error: Missing definition for abstract method `MyAbstractClass#example` in `MyClass`
end

# Multi-line format
class MultiParamAbstractClass
  extend T::Sig, T::Helpers
  abstract!

  sig {abstract.type_parameters(:U).params(a: T.type_parameter(:U), b: Integer, c: String, d: Float, e: Symbol).returns(T.type_parameter(:U))}
  def example(a, b, c, d, e); end
end

class MultiParamConcreteClass < MultiParamAbstractClass
# error: Missing definition for abstract method `MultiParamAbstractClass#example` in `MultiParamConcreteClass`
end
