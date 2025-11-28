# typed: false

# Edge cases for 'it' parameter that would confuse string-based location finding
# but are handled correctly by AST walking

# Case 1: 'it' in comments should not affect the block parameter
result1 = [1, 2, 3].map do
  # This comment mentions it but shouldn't affect anything
  it * 2  # The actual 'it' parameter usage
end

# Case 2: 'it' in string literals
result2 = [1, 2, 3].map { "it is: #{it}" }

# Case 3: Multiple occurrences of 'it' in different contexts
result3 = [1, 2, 3].map do
  # This comment also mentions the word it
  str = "it works"
  it + 1  # Actual parameter usage
end
