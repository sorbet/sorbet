# typed: true

class A
  extend T::Sig
  extend T::Generic
  Elem = type_member
  AnotherElem = type_member

  AliasToElem = Elem

  sig {params(x: AliasToElem).void}
  def example(x)
    T.reveal_type(x) # error: `A::Elem`
  end
end
