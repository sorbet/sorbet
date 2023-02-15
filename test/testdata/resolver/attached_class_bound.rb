# typed: strict

class Wrapper
  extend T::Sig
  extend T::Generic

  X = type_member(:out) { {upper: ParentThing} }

  sig {params(x: X).void}
  def initialize(x)
    @x = x
  end
end

class ParentThing
  extend T::Sig
  extend T::Generic

  # ParentThing::<AttachedClass>   <:   ParentThing
  #
  # so
  #
  # Wrapper[ParentThing::<AttachedClass>]   <:   Wrapper[ParentThing]

  sig {void}
  def self.example
    x = self.new
    T.reveal_type(x) # error: `T.attached_class (of ParentThing)`
    y = T.let(x, ParentThing)

    ex = Wrapper[T.attached_class].new(x)
    T.reveal_type(ex) # error: `Wrapper[T.attached_class (of ParentThing)]`
    ex2 = T.let(ex, Wrapper[ParentThing])
  end

  sig {returns(Wrapper[T.attached_class])}
  def self.make_thing_and_wrap
    thing = self.new
    T.reveal_type(thing) # error: `T.attached_class (of ParentThing)`
    x = Wrapper[T.attached_class].new(thing)
    p(x)
    x
  end
end

class ChildThing < ParentThing
end

parent_thing = ParentThing.make_thing_and_wrap
T.reveal_type(parent_thing) # error: `Wrapper[ParentThing]`

child_thing = ChildThing.make_thing_and_wrap
T.reveal_type(child_thing) # error: `Wrapper[ChildThing]`

class Unrelated
  extend T::Sig
  extend T::Generic

  sig {returns(Wrapper[T.attached_class])}
  #                    ^^^^^^^^^^^^^^^^ error: `T.attached_class (of Unrelated)` is not a subtype of upper bound of type member `::Wrapper::X
  def self.foo
    unrelated = self.new
    x = Wrapper[T.attached_class].new(unrelated)
    #           ^^^^^^^^^^^^^^^^ error: `T.attached_class (of Unrelated)` is not a subtype of upper bound of type member `::Wrapper::X
    p(x)
    x
  end
end
