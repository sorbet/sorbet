# typed: true

class A
  def initialize
    @var = T.let(0, Integer)
  end
end
