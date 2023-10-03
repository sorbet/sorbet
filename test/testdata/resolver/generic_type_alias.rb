# typed: true
class Module; include T::Sig; end

class Box
  extend T::Generic
  Elem = type_member

  sig { params(val: Elem).void }
  def initialize(val); @val = val; end

  sig { returns(Elem) }
  def val; @val; end
end

class ComplicatedBox < Box
  ReturnType = T.type_alias do
    T.any(Integer, String, Float, Symbol)
  end
  Elem = type_member { {fixed: ReturnType} }
end

sig { params(box: ComplicatedBox).returns(ComplicatedBox::ReturnType) }
def example_good3(box)
  T.reveal_type(box.val) # error: `T.any(Integer, String, Float, Symbol)`
  nil # error: Expected `T.any(Integer, String, Float, Symbol)` but found `NilClass` for method result type
end

class BoxBad
  extend T::Generic
  abstract!

  Elem = type_member
  MyElem = T.type_alias { Elem } # error: Defining a `type_alias` to a generic `type_member` is not allowed
  MyElem2 = T.type_alias { T.nilable(Elem) } # error: Defining a `type_alias` to a generic `type_member` is not allowed

  sig { abstract.returns(Elem) }
  def returns_elem; end

  sig { returns(MyElem) }
  def returns_my_elem
    T.let(returns_my_elem, Elem)
    T.let(returns_my_elem, MyElem)
    my_elem = returns_my_elem
    T.reveal_type(my_elem) # error: `T.untyped`
    nil
  end

  sig { returns(MyElem) }
  def self.returns_my_elem
    T.let(
      returns_my_elem,
      Elem  # error: `type_member` type `Elem` used in a singleton method definition
    )
    T.let(
      returns_my_elem,
      MyElem
    )
    my_elem = returns_my_elem
    T.reveal_type(my_elem) # error: `T.untyped`
    nil
  end
end

sig do
  params(
    x: BoxBad::Elem,
    #  ^^^^^^^^^^^^ error: `type_member` type `BoxBad::Elem` used outside of the class definition
    y: BoxBad::MyElem,
    z: BoxBad::MyElem2
  )
    .void
end
def bad_box_example(
  x,
  y,
  z
)
  T.reveal_type(x) # error: `T.untyped`
  T.reveal_type(y) # error: `T.untyped`
  T.reveal_type(z) # error: `T.untyped`
end

