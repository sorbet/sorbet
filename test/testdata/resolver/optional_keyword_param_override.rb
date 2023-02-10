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
  def foo(x:) # error: Implementation of overridable method `Left#foo` contains extra required keyword argument `x`
  end
end
