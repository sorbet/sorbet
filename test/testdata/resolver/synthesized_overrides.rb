# typed: true

class A; end
class B < A; end

class Unrelated; end

class ReaderParent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.returns(A) }
  def a; end
end

# Covariance rule for attr readers return types should be checked

class CovariantReader < ReaderParent
  extend T::Sig

  sig { override.returns(B) } # Ok, covariance
  attr_reader :a

  sig { params(b: B).void }
  def initialize(b)
    @a = T.let(b, B)
  end
end

class NonCovariantReader < ReaderParent
  extend T::Sig

  sig { override.returns(T.nilable(Object)) }
  attr_reader :a
# ^^^^^^^^^^^^^^ error: Return type `Object` does not match return type of abstract method `ReaderParent#a`

  sig { void }
  def initialize
    @a = T.let(nil, T.nilable(Object))
  end
end

class NilableReader < ReaderParent
  extend T::Sig

  sig { override.returns(T.nilable(B)) }
  attr_reader :a
# ^^^^^^^^^^^^^^ error: Return type `T.nilable(B)` does not match return type of abstract method `ReaderParent#a`

  sig { void }
  def initialize
    @a = T.let(nil, T.nilable(B))
  end
end

class UnrelatedReader < ReaderParent
  extend T::Sig

  sig { override.returns(Unrelated) }
  attr_reader :a
# ^^^^^^^^^^^^^^ error: Return type `Unrelated` does not match return type of abstract method `ReaderParent#a`

  sig { params(a: Unrelated).void }
  def initialize(a)
    @a = T.let(a, Unrelated)
  end
end

class ManualReader < ReaderParent
  extend T::Sig

  sig { override.returns(B) } # Ok
  def a
    B.new
  end
end

class WriterParent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.params(a: A).void }
  def a=(a); end
end

class AttrWriter < WriterParent
  extend T::Sig

  sig { override.params(a: A).void } # Ok
  attr_writer :a
end

class NonContravariantWriter < WriterParent
  extend T::Sig

  sig { override.params(a: B).void } # Bad, B should be supertype of A
  #                     ^ error: Parameter `a` of type `B` not compatible with type of abstract method `WriterParent#a=`
  attr_writer :a
end

class AccessorParent
  extend T::Sig

  sig { returns(A) }
  attr_accessor :a
end

class ReaderOnly < AccessorParent
  extend T::Sig

  sig { override.returns(B) }
  attr_reader :a
end

class WriterOnly < AccessorParent
  extend T::Sig

  sig { override.params(a: A).returns(A) }
  attr_writer :a
end

class ReaderWriterParent
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.returns(A) }
  def a; end

  sig { abstract.params(x: A).returns(A) }
  def a=(x); end
end

class AttrChild < ReaderWriterParent
  extend T::Sig

  sig { override.returns(A) }
  attr_accessor :a
end
