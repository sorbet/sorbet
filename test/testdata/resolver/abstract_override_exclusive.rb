# typed: false

class AbstractGrandParent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.returns(Object)}
  def foo; end

  sig {abstract.returns(Object)}
  def bar; end
end

class AbstractParent < AbstractGrandParent
  abstract!

  sig {abstract.override.returns(T::Boolean)} # error: `abstract` cannot be combined with `override`
  def foo; end

  sig {override.abstract.returns(T::Boolean)} # error: `override` cannot be combined with `abstract`
  def bar; end
end

class ConcreteChild < AbstractParent
  sig {override.returns(T::Boolean)}
  def foo
    true
  end

  sig {override.returns(T::Boolean)}
  def bar
    true
  end
end

ConcreteChild.new.foo
