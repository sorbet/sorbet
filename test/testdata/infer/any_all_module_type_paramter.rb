# typed: true
extend T::Sig

module M; end
class A; end

sig do
  type_parameters(:T)
    .params(xs: T::Array[T.any(A, T.all(T.type_parameter(:T), M))])
    .returns(T.nilable(T.any(A, T.all(T.type_parameter(:T), M))))
end
def self.foo(xs)
  xs.find do |x|
    if x.is_a?(A)
      #  ^^^^^ error: `is_a?` does not exist on `Object.foo#T` component
      #  ^^^^^ error: `is_a?` does not exist on `M` component
      puts x
    end
  end
end
