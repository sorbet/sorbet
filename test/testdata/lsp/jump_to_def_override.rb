# typed: true

class Parent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def example1
# ^ def: Child#example1
  end

  sig {overridable.void}
  def example2
# ^ def: Child#example2
  end
end

class Child < Parent
  sig {override.void}
  #    ^ go-to-def-special: Child#example1
  def example1
  end

  sig {override.void}
  #    ^ go-to-def-special: Child#example2
  def example2
  end
end
