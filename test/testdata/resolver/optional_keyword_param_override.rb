# typed: strict
extend T::Sig

class Left
  extend T::Sig

  sig { overridable.params(x: T::Boolean).returns(T.nilable(String)) }
  def foo(x: false)
  end
end

class Right1 < Left
  sig { override.returns(T.nilable(String)) }
  def foo # error: Implementation of overridable method `Left#foo` must accept optional keyword parameter `x`
  end
end
class Right2 < Left
  sig { override.params(x: T::Boolean).returns(T.nilable(String)) }
  def foo(x: false)
  end
end
class Right3 < Left
  sig { override.params(x: T::Boolean).returns(T.nilable(String)) }
  def foo(x:) # error: Implementation of overridable method `Left#foo` must redeclare keyword parameter `x` as optional
  end
end

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.params(no_check: T::Boolean).void}
  def example(no_check: false)
  end
end

class Child1 < Parent
  sig {override.params(opts: T::Hash[T.untyped, T.untyped]).void}
  def example(opts={}) # error: Implementation of abstract method `Parent#example` must accept optional keyword parameter `no_check`
  end
end
class Child2 < Parent
  # The runtime doesn't allow this
  sig {override.params(opts: T.untyped).void}
  def example(**opts)
  end
end
