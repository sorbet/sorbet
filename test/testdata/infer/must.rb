# typed: strict

def test_must # error: does not have a `sig`
  x = T.cast(nil, T.nilable(String)) # error: `T.cast` is useless
  T.assert_type!(T.must(x), String)

  T.must(x)
  T.must()  # error: Not enough arguments
  T.must(x, 0)
  #         ^ error: Expected: `1`, got: `2`
end
