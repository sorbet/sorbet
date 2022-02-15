# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def example1(x)
  x.foo # error: Call to method `foo` on unconstrained generic type `Object#example1#U`
end

module M
  def foo; end
end

sig do
  type_parameters(:U)
    .params(x: T.all(T.type_parameter(:U), M))
    .void
end
def example2(x)
  x.foo
end


sig do
  type_parameters(:U)
    .params(x: T.any(T.type_parameter(:U), M))
    .void
end
def example3(x)
  x.foo # error: Call to method `foo` on generic type `Object#example3#U` component of `T.any(M, Object#example3#U)`
end
