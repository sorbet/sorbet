# typed: strict

class A
  extend T::Sig

  @x = T.let(0, Integer)
  @y = T.let("", String)

  sig {returns(String)}
  def self.y
    @y
  end
end
