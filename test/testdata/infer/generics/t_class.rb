# typed: strict
extend T::Sig

class A; end

sig do
  type_parameters(:U)
    .params(klass: T::Class[T.type_parameter(:U)])
    .returns(T.type_parameter(:U))
end
def instantiate_class(klass)
  instance = klass.new
  T.reveal_type(instance) # error: `T.type_parameter(:U) (of Object#instantiate_class)`
  instance
end

a = instantiate_class(A)
T.reveal_type(a) # error: `A`
