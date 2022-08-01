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
    T.reveal_type(klass) # error: `T.class_of(Box)`
    new_box = self.class.new(new_val)
    T.reveal_type(new_box) # error: `Box[T.untyped]`

    box_elem_class = self.class[Elem]
    T.reveal_type(box_elem_class) # error: Runtime object representing type: Box[Box::Elem]
    box_elem = box_elem_class.new(new_val)
    T.reveal_type(box_elem) # error: Box[Box::Elem]

    self.class[Elem].new('')
    #                    ^^ error: Expected `Box::Elem` but found `String("")` for argument `val`

    box_elem
  end
end

box = Box[Integer].new(0).copy_with(1)
T.reveal_type(box) # error: `Box[Integer]`
