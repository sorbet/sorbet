# typed: true

# This actually works at runtime too, miraculously, without any additional work.

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {params(val: Elem).void}
  def initialize(val)
    @val = val
  end

  sig {params(new_val: Elem).returns(T.self_type)}
  def copy_with(new_val)
    klass = self.class
    T.reveal_type(klass) # error: `T.class_of(Box)[Box[Box::Elem]]`
    new_box = self.class.new(new_val)
    T.reveal_type(new_box) # error: `Box[Box::Elem]`

    box_elem_class = self.class[Elem]
    T.reveal_type(box_elem_class) # error: Runtime object representing type: Box[Box::Elem]
    box_elem = box_elem_class.new(new_val)
    T.reveal_type(box_elem) # error: Box[Box::Elem]

    self.class[Elem].new('')
    #                    ^^ error: Expected `Box::Elem` but found `String("")` for argument `val`

    box_elem
  end

  # Sorbet doesn't check this statically yet. Once it does, this will cause
  # problems for some of the tricks we're using here (unfortunately, with no
  # workarounds except `T.untyped`)
  private_class_method :new

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .returns(T.all(T.attached_class, Box[T.type_parameter(:U)]))
  end
  def self.make(x)
    # "works" but only because of T.untyped
    res = new(x)
    T.reveal_type(res) # error: `T.attached_class (of Box)`

    # No error for using Integer because, again, it's T.untyped
    res = new(0)
    T.reveal_type(res) # error: `T.attached_class (of Box)`

    # Wild that this syntax works, even at runtime!
    # Though, it's a non-private call, so you have to either pick private or typed :/
    res = self[T.type_parameter(:U)].new(x) # !!
    T.reveal_type(res) # error: `T.all(Box[T.type_parameter(:U) (of Box.make)], T.attached_class (of Box))`

    self[T.type_parameter(:U)].new(0)
    #                              ^ error: Expected `T.type_parameter(:U) (of Box.make)` but found `Integer(0)` for argument `val`

    new(x)
  end
end

class ChildBox < Box
  Elem = type_member
end

box = Box[Integer].new(0).copy_with(1)
T.reveal_type(box) # error: `Box[Integer]`

box = Box.make('')
T.reveal_type(box) # error: `Box[String]`
box = ChildBox.make('')
T.reveal_type(box) # error: `ChildBox[String]`
