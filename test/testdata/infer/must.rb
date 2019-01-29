# typed: strict

def test_must # error: does not have a `sig`
  x = T.cast(nil, T.nilable(String)) # error: Useless cast
  T.assert_type!(T.must(x), String)

  T.must(x, "hi")
  T.must()  # error: Not enough arguments
  T.must(x, "hi", 0)  # error: Expected: `1..2`, got: `3`
  T.must(x, :foo)  # error: `Symbol(:"foo")` doesn't match `T.nilable(String)` for argument `msg`
end
