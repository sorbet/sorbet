# typed: true
def test_large_int
  [
    0B, # parser-error: numeric literal without digits
    0X, # parser-error: numeric literal without digits
    00,
    9223372036854775807,
    -9223372036854775808,
    18446744073709551616,   # error: Unsupported integer
    0xcafe,
    0_0,
    1_, # parser-error: trailing `_`
  ]
end
