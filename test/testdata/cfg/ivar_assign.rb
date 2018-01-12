# @typed
class TestIVar
  declare_variables(:@foo => Integer)

  def test
    @foo = nil # error: NilClass is not a subtype of Integer
  end
end
