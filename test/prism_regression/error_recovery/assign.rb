# typed: false
# Should still see at least method def (not body)
def test_bad_assign(x)
  x = # error: unexpected token
end
