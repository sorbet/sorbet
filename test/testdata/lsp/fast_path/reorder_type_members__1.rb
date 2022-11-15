# typed: strict

class Parent
  extend T::Sig
  extend T::Generic

  Elem1 = type_member
  Elem2 = type_member

  sig {params(x: Elem1, y: Elem2).returns(Elem1)}
  def example(x, y)
    T.reveal_type(x) # error: `Parent::Elem1`
    T.reveal_type(y) # error: `Parent::Elem2`

    x
  end
end
