# typed: true

class A
  extend T::Sig, T::Generic
  Elem = type_member
  sig { params(x: self.A).void }
                # ^^^^^^ error: Malformed type declaration
                #      ^ error: Method `A` does not exist
  def test(x); end
end
