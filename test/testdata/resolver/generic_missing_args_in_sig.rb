# typed: true

class A
  extend T::Sig, T::Generic
  Elem = type_member
  sig { params(x: self.A).void }
  def test(x); end
end
