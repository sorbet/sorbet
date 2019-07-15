# typed: true
def test_large_int
  [
    0B, # error-with-dupes: numeric literal without digits
    0X, # error-with-dupes: numeric literal without digits
    00,
    9223372036854775807,
    -9223372036854775808,
    18446744073709551616,   # error-with-dupes: Unsupported integer
    0xcafe,
    0_0,
    1_, # error-with-dupes: trailing `_`
  ]
end
