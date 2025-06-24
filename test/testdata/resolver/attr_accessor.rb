# typed: true

class A; end
class B < A; end

class Unrelated; end

class Foo
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.returns(A)}
  def a; end
end

# Covariance rule for attr readers return types should be checked:

class Error1 < Foo
  extend T::Sig

  sig { override.returns(Unrelated)}
  attr_reader :a
# ^^^^^^^^^^^^^^ error: Return type `Unrelated` does not match return type of abstract method `Foo#a`

  sig { params(a: Unrelated).void }
  def initialize(a)
    @a = T.let(a, Unrelated)
  end
end

class Error2 < Foo
  extend T::Sig

  sig { override.returns(T.nilable(Object))}
  attr_reader :a
# ^^^^^^^^^^^^^^ error: Return type `Object` does not match return type of abstract method `Foo#a`

  sig { void }
  def initialize
    @a = T.let(nil, T.nilable(Object))
  end
end

class Error3 < Foo
  extend T::Sig

  sig { override.returns(T.nilable(B))}
  attr_reader :a
# ^^^^^^^^^^^^^^ error: Return type `T.nilable(B)` does not match return type of abstract method `Foo#a`

  sig { void }
  def initialize
    @a = T.let(nil, T.nilable(B))
  end
end

# The following should still pass

class Ok1 < Foo
  extend T::Sig

  sig { override.returns(B)} # Ok, covariance
  attr_reader :a

  sig { params(b: B).void }
  def initialize(b)
    @a = T.let(b, B)
  end
end

class Ok2 < Foo
  extend T::Sig

  sig { override.returns(B)} # Ok
  def a
    B.new
  end
end
