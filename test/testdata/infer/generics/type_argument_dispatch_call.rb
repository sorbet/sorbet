# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def example1(x)
  x.foo # error: Call to method `foo` on unconstrained generic type `T.type_parameter(:U) (of Object#example1)`
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
  x.foo # error: Call to method `foo` on generic type `T.type_parameter(:U) (of Object#example3)` component of `T.any(M, T.type_parameter(:U) (of Object#example3))`
end
