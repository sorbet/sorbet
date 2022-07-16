# typed: true
extend T::Sig

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {returns(Elem)}
  attr_accessor :val

  sig {params(val: Elem).void}
  def initialize(val)
    @val = val
  end

  sig do
    type_parameters(:U)
      .params(val: T.type_parameter(:U))
      .returns(Box[T.type_parameter(:U)])
  end
  def self.make(val)
    x = new(val)
    T.reveal_type(x)
    x
  end

  sig do
    type_parameters(:U)
      .params(val: T.type_parameter(:U))
      .returns(T.attached_class)
  end
  def self.make2(val)
    x = new(val)
    T.reveal_type(x)
    x
  end
end

x = Box.new(0)
T.reveal_type(x)
x = Box.make(0)
T.reveal_type(x)


sig do
  type_parameters(:U)
    .params(x: Box[T.type_parameter(:U)])
    .returns(T.type_parameter(:U))
end
def example(x)
  x.val
end

res = example(Box[Integer].new(0))
T.reveal_type(res)

x = Box.make2(0)
T.reveal_type(x)
