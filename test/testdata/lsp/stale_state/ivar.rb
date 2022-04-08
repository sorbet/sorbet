# typed: true

class A
  def initialize
    @x = T.let(1, Integer)
  end
end
