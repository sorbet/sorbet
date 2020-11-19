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

class Child < Parent
  # TODO(jez) This is just unimplemented
  private :abstract_in_grandparent # error: Can't narrow visibility of abstract method `abstract_in_grandparent`

  private :overridable_in_grandparent # error: Can't narrow visibility of overridable method `overridable_in_grandparent`
end
