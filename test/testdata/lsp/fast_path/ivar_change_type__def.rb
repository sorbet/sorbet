# typed: true

class Parent
  def initialize
    @x = T.let(0, Integer)
  end
end
