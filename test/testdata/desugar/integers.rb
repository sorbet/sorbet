def test_large_int
  [
    9223372036854775807,
    -9223372036854775808,
    18446744073709551616,   # error: Unsupported large integer
    0xcafe, # error: Unsupported integer
  ]
end
