# typed: true
# See https://github.com/sorbet/sorbet/issues/3761 -- file order matters for surfacing bug.
class Foo
  extend T::Sig

  def initialize
    @a = T.let(Suit::Spades, Suit::Spades)
  end

  sig{params(a: Suit::Spades).returns(T::Boolean)}
  def foo(a)
    true
  end

end
