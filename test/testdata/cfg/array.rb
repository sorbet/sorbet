# @typed
class TestArray
  standard_method(returns: Integer)
  def an_int; 0; end

  standard_method(returns: String)
  def a_string; 'str'; end

  def test_arrays
    []
    [1, 2]
    [an_int, a_string, []]
  end
end
