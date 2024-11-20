# typed: false
# Should still see at least method def (not body)
def test_bad_assign(x)
  x =
  # ^ error: expected an expression after `=`
end # error: unexpected 'end', assuming it is closing the parent method definition
