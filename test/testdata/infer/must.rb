# @typed

def test_must
  x = T.cast(nil, T.nilable(String))
  T.assert_type!(T.must(x), String)

  T.must(x, "hi")
  T.must()  # error: Wrong number of arguments
  T.must(x, "hi", 0)  # error: Wrong number of arguments
  T.must(x, :foo)  # error: Expression passed as an argument `msg` to method `must` does not match expected type `String`
end
