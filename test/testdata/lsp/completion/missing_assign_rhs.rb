# typed: true

class A
  def foo; end
  def test_missing_rhs(x)
    y =
    #  ^ completion: x, y, foo, test_missing_rhs, ...
  end # error: unexpected token "end"
end
