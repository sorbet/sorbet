# typed: true
class Foo
  extend T::Sig

  sig { params(x: Integer, y: Integer).void }
  def initialize(x, y)
    @x = x
    @y = y + 1
  end

  sig { void }
  def foo
    T.reveal_type(@x) # error: Revealed type: `Integer`
    T.reveal_type(@y) # error: Revealed type: `T.untyped`
  end
end
