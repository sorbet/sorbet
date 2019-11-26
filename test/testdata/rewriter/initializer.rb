# typed: true
class Foo
  extend T::Sig

  sig { params(x: Integer).void }
  def initialize(x)
    @x = x  # this will get a synthetic `T.let`
    @y = x + 1  # This should not, because the RHS is not a simple local
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
    T.reveal_type(@y) # error: Revealed type: `T.untyped`
  end
end

class Bar
  extend T::Sig

  # we should find `params` correctly here
  sig { overridable.params(x: Integer).overridable.void }
  def initialize(x)
    @x = x
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
  end
end

class Baz
  extend T::Sig

  # nobody should ever write this, but just in case, we should make
  # sure not to include type_parameter types in a generated `T.let`
  sig { type_parameters(:U).params(x: T.type_parameter(:U)).void }
  def initialize(x)
    @x = x
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `T.untyped`
  end
end
