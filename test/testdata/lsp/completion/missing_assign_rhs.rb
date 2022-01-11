# typed: true

def test_missing_rhs(x)
  y =
  #  ^ completion: (nothing)
end # error: unexpected token "end"
