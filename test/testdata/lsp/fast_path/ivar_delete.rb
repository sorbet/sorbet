# typed: strict

class A
  extend T::Sig

  sig {void}
  def initialize
    @x = T.let(0, Integer)
    @y = T.let("", String)
  end

  sig {returns(String)}
  def y
    @y
  end
end
