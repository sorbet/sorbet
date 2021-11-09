# typed: true
class TestIVar
  def initialize
    @foo = T.let(0, Integer)
  end

  def test
    @foo = nil # error: Expected `Integer` but found `NilClass` for field
  end
end
