# typed: true

# Test 'it' as a soft keyword: can be reassigned, used as variable name, method parameter, etc.
# Based on Ruby 3.4 spec

# 'it' can be reassigned within a block (changes from block param to local var)
[1, 2, 3].each {
  x = it       # x is Integer (from block parameter)
  it = "str"   # Reassign 'it' to String (this is allowed!)
  y = it.upcase # y is String, upcase exists on String
  z = it + 2    # error: Expected `String` but found `Integer(2)` for argument `arg0`
}

# Reassignment with do...end syntax
[1, 2, 3].each do
  # Initially 'it' is Integer (block parameter)
  x = it

  # Assign 'it' to a String (now it's a local variable)
  it = "reassigned"

  # Type error: 'it' is now String, not Integer
  it + 3 # error: Expected `String` but found `Integer(3)` for argument `arg0`

  # Can use String methods
  it.upcase
end

# Compare to _1 which cannot be reassigned
[1, 2, 3].each {
  _1 = "str"   # error: _1 is reserved for numbered parameter
}

# 'it' can be used as a regular local variable outside blocks
it = 42
x1 = it + 10

it = "string"
x2 = it.upcase

# When 'it' is defined as local variable, it shadows block parameter
it = [10, 20, 30]
result = [1, 2, 3].map { it.length }
# 'it' refers to the array [10, 20, 30], not the block parameter

# Type error: 'it' is Array, not Integer
[1, 2, 3].map { it + 2 } # error: Expected `T::Enumerable[T.type_parameter(:T)]` but found `Integer(2)` for argument `arg0`

# 'it' can be used as method parameter name (soft keyword behavior)
def process(it)
  it
end

x = process(42)

# Unlike _1, 'it' doesn't conflict with outer scope in nested blocks
# Each block gets its own 'it'
[[1, 2], [3, 4]].each do
  outer_it = it  # Array

  it.each do
    inner_it = it  # Integer
    # No conflict - inner 'it' shadows outer 'it' within this block
  end
end

# Comparison with _1: nested blocks cause issues
[[1, 2], [3, 4]].each do
  # With _1, inner and outer would conflict:
  # _1.each { _1 } # Would refer to outer _1, not inner element

  # With 'it', each block has its own:
  it.each { it * 2 } # Inner 'it' is different from outer 'it'
end
