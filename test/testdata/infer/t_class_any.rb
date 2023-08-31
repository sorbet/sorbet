# typed: true
extend T::Sig

class A; end
class B; end

sig do
  type_parameters(:Instance)
    .params(klass: T::Class[T.type_parameter(:Instance)])
    .returns(T.type_parameter(:Instance))
end
def instantiate_class(klass)
  instance = klass.new
  puts("Instantiated: #{instance}")
  instance
end

sig {params(klass: T::Class[T.any(A, B)]).void}
def instantiate_class_a_b(klass)
  instance = klass.new
  T.reveal_type(instance) # error: `T.any(A, B)`
  case instance
  when A
  when B
  else
    T.absurd(instance)
  end
end


sig {params(klass: T.any(T.class_of(A), T.class_of(B))).void}
def example(klass)
  x = instantiate_class(klass)
  T.reveal_type(x) # error: `T.any(A, B)`
  x
end
