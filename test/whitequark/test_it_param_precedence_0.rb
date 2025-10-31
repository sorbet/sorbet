# typed: true

# Test precedence: local variable 'it' takes precedence over block parameter
it = "string"
result = [1, 2, 3].map { it.upcase } # Should use the local variable 'it' which is "string"
# Result is ["STRING", "STRING", "STRING"]

# After the block, 'it' still refers to local variable
puts it # prints "string"
