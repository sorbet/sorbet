# typed: false

# Test that 'it' can be assigned to (soft keyword behavior)
# Unlike _1, which cannot be assigned to
[1, 2, 3].each {
  x = it
  it = x + 1
  puts it
}
