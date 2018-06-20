# typed: strict

def test_must
  x = T.cast(nil, T.nilable(String)) # error: Useless cast
  T.assert_type!(T.must(x), String)

  T.must(x, "hi")
  T.must()  # error: Not enough arguments
  T.must(x, "hi", 0)  # error: Too many arguments
  T.must(x, :foo)  # error: `Symbol(:"foo")` doesn't match `String` for argument `error`
end
