# typed: true

class A
  extend T::Sig

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .returns(T.type_parameter(:U))
  end
  def id(x)
    T.reveal_type(x) # error: `T.type_parameter(:U) (of A#id)`
  end
end
