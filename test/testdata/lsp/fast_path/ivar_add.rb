# typed: true

class A
  extend T::Sig

  sig {void}
  def initialize
    @x = T.let("", String)
  end

  sig {void}
  def example
    T.reveal_type(@x) # error: Revealed type: `String`
  end
end
