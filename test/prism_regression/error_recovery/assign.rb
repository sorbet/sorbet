# typed: false
# disable-parser-comparison: true
# Should still see at least method def (not body)
def test_bad_assign(x)
  x =
  # ^ error: expected an expression after `=`
end
