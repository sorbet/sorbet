# typed: false
# Should still see at least method def (not body)
def test_bad_assign(x)
  x =
end # parser-error: unexpected token
