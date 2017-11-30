# @typed
A, B, C = some_array

class Test
  def some_method(array)
    a, b, (c, d) = array  # error: Unsupported node
  end
end
