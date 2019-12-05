# typed: true

class Func
  extend T::Sig
  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .returns(T.type_parameter(:U))
  end
  def self.id(x)
    x
  end
end
