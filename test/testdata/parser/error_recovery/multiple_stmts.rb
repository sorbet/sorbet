# typed: false

def test_method_with_multiple_stmts
  x = 1
  x = ; # error: unexpected token
  y = 1
end
