# typed: true
def test_large_int
  [
    9223372036854775807,
    -9223372036854775808,
    18446744073709551616,   # error: Unsupported integer
    0xcafe,
    0_0,
    1_, # error: trailing `_`
  ]
end
