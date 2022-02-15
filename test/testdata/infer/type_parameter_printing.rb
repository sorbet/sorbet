# typed: true

class A
  extend T::Sig

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .void
  end
  def example1(x)
    T.reveal_type(x) # error: `T.type_parameter(:U) (of A#example1)`
  end

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .void
  end
  def self.example2(x)
    T.reveal_type(x) # error: `T.type_parameter(:U) (of A.example2)`
  end
end
