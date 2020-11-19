# typed: false

class GrandParent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def abstract_in_grandparent; end

  sig {overridable.void}
  def overridable_in_grandparent; end
end

class Parent < GrandParent
  sig {override.void}
  def abstract_in_grandparent; end

  sig {override.void}
  def overridable_in_grandparent; end
end

# Would like to emit errors here, but we don't do this for normal `abstract/override/override`
# chains either, so I didn't bother to implement it for ZSuper methods.
class Child < Parent
  private :abstract_in_grandparent # would like to be error: Can't narrow visibility

  private :overridable_in_grandparent # would like to be error: Can't narrow visibility
end
