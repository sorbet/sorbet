# typed: strict
extend T::Sig

# All of the examples in this file showcase current limitations in how we've
# chosen to implement the requirement that a generic class be provided type
# arguments on instantiation.
#
# The check is currently syntactic, which means it fails in ways that you might
# expect Sorbet to be able to detect using full inference.
#
# This is a tradeoff, because implementing something was better than nothing.

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {returns(T.attached_class)}
  def self.make
    res = self.new
    T.reveal_type(res) # error: `T.attached_class (of Box)`
    res = new
    T.reveal_type(res) # error: `T.attached_class (of Box)`
    res
  end

  sig {returns(T.self_type)}
  def another
    res = self.class.new
    T.reveal_type(res) # error: `Box[T.untyped]`
    res
  end
end

sig {params(box_class: T.class_of(Box)).void}
def make_box(box_class)
  box = box_class.new
  T.reveal_type(box) # error: `Box[T.untyped]`
end

sig {void}
def example
  box_class = Box
  box = box_class.new
  T.reveal_type(box) # error: `Box[T.untyped]`

  box = Box.make
  T.reveal_type(box) # error: `Box[T.untyped]`

  box2 = box.another
  T.reveal_type(box2) # error: `Box[T.untyped]`
end
