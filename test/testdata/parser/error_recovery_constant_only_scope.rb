# typed: false
# Should still see at least method def (not body)
def test_constant_only_scope
  A::
end # error: unexpected token
