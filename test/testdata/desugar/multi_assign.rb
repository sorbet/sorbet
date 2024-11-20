# typed: false
A, B, C = some_array

class Test
  def some_method(array)
    a, b, (c, d) = array
    a[b], c = array
    a, *b = array
    *a, b = array
    a, b, *c, d, e = array
    a, * = array
    a.x, b = array
    a, *b = *T.unsafe(array)
    *a, b = *T.unsafe(array)
  end
end
