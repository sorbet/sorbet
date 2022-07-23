# typed: true
extend T::Sig

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig { params(v: Elem).void }
  def initialize(v)
  end
end

sig do
  type_parameters(:T)
  .params(v: T.type_parameter(:T))
  .returns(Box[T.type_parameter(:T)])
end
def build_a(v)
  Box.new(v)
end

sig { params(v: Box[Integer]).void }
def test_int(v)
end

a = build_a(42)
T.reveal_type(a) # error: `Box[Integer]`
test_int(a)
