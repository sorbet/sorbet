# typed: true

class Parent
  extend T::Sig
  sig {overridable.void}
  def foo; end
end

module IChild
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def bar; end
end

class Child < Parent
  include IChild
  sig {void}
  def foo; end

  sig {void}
  def bar; end
end
