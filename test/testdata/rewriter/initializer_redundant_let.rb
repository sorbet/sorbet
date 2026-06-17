# typed: true
# allow-redundant-t-let-in-initialize: false
class Simple
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    @x = T.let(x, Integer) # error: Redundant `T.let` in `initialize`
  end
end

class DifferentType
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    @x = T.let(x, T.nilable(Integer)) # no error: type differs from what rewriter would infer
  end
end

class NotAParam
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    y = x
    @y = T.let(y, Integer) # no error: y is not a param
  end
end

class MultipleParams
  extend T::Sig

  sig { params(x: Integer, y: String).void }
  def initialize(x, y)
    @y = T.let(y, String) # error: Redundant `T.let` in `initialize`
    @x = T.let(x, Integer) # error: Redundant `T.let` in `initialize`
  end
end

class NotLet
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    @x = T.cast(x, Integer) # error: Use `T.let` to specify the type of instance variables
  end
end

class BareAssignment
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    @x = x # no error: no T.let (will get synthetic one from rewriter)
  end
end

class NilableType
  extend T::Sig

  sig { params(x: T.nilable(String)).void }
  def initialize(x)
    @x = T.let(x, T.nilable(String)) # error: Redundant `T.let` in `initialize`
  end
end
