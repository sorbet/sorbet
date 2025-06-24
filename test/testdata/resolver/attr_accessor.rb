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

# Covariance rule for attr readers return types is not checked:

class Error1 < Foo
  extend T::Sig

  sig { override.returns(Unrelated)} # WRONG: Should error, as Unrelated is not a subtype of A
  attr_reader :a

  sig { params(a: Unrelated).void }
  def initialize(a)
    @a = T.let(a, Unrelated)
  end
end

class Error2 < Foo
  extend T::Sig

  sig { override.returns(T.nilable(Object))} # WRONG: Should error, as nilable Object is not covariant with A
  attr_reader :a

  sig { void }
  def initialize
    @a = T.let(nil, T.nilable(Object))
  end
end

class Error3 < Foo
  extend T::Sig

  sig { override.returns(T.nilable(B))} # WRONG: Should error, as nilable B is not covariant with A
  attr_reader :a

  sig { void }
  def initialize
    @a = T.let(nil, T.nilable(B))
  end
end

# The following tests should still work once fixed:

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

# This already work well with concrete methods:

class Ok3 < Foo
  extend T::Sig

  sig { override.returns(T.nilable(B))} # Ok: errors as it should
  def a
    B.new
  end
end

class Ok4 < Foo
  extend T::Sig

  sig { override.returns(Object)} # Ok: errors as it should
  def a
    B.new
  end
end

class Ok5 < Foo
  extend T::Sig

  sig { override.returns(Unrelated)} # Ok: errors as it should
  def a
    Unrelated.new
  end
end
