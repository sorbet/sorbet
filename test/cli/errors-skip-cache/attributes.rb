# typed: true

class A
  extend T::Sig

  attr "bad attribute name"

  sig { type_parameters(:K).returns(T.type_parameter(:K)) }
  attr :baz
end
