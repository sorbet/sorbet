# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def example1(x)
  x.foo # error: Call to method `foo` on unconstrained generic type `Object#example1#U`

  if x.is_a?(Integer)
    T.reveal_type(x)
  else
    T.reveal_type(x)
  end

  if x.nil?
    T.reveal_type(x)
  else
    T.reveal_type(x)
  end
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
  x.foo # error: Method `foo` does not exist on `Object#example3#U` component of `T.any(M, Object#example3#U)`
end

sig do
  type_parameters(:U)
    .params(xs: T::Array[T.type_parameter(:U)])
    .void
end
def example4(xs)
  xs.map(&:foo) # error: Method `foo` does not exist on `Object#example4#U`
end
