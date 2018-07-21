# typed: false
def test_csend
  foo&.bar
  foo&.bar { |x| x }
end
