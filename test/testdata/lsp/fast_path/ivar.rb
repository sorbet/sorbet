# typed: true

class A
  extend T::Sig

  sig {void}
  def initialize
    @x = T.let(0, Integer)
  end

  sig {void}
  def example
    T.reveal_type(@x) # error: `Integer`
  end
end
