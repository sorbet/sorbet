# @typed
class TestIVar
  def initialize
    @foo = T.let(0, Integer)
  end

  def test
    @foo = nil # error: NilClass is not a subtype of Integer
  end
end
