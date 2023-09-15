# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(x: @U)
    .returns(@U)
end
def foo(x)
  T.reveal_type(x)
  ''
end

p T::Utils.signature_for_method(method(:foo))
