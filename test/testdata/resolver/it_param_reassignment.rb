# typed: true

# Test that 'it' can be reassigned within block (soft keyword)
# and type checking follows the reassignment

[1, 2, 3].each {
  x = it       # x is Integer (from block parameter)
  it = "str"   # Reassign 'it' to String (this is allowed!)
  y = it.upcase # y is String, upcase exists on String
  z = it + 2    # error: Expected `String` but found `Integer(2)` for argument `arg0`
}

# Compare to _1 which cannot be reassigned
[1, 2, 3].each {
  _1 = "str"   # error: _1 is reserved for numbered parameter
}
