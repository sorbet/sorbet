# typed: true

# Test that 'it' in nested blocks refers to the innermost block parameter
# and each 'it' has its own type

# Nested blocks with different types - outer 'it' is Array, inner 'it' is Integer
[[1, 2], [3, 4]].each do
  # Outer 'it' is an Array
  outer_length = it.length # Arrays have length

  it.each do
    # Inner 'it' is Integer
    is_zero = it.zero? # Integer has .zero? (Array doesn't)

    # Type error: Integer doesn't have upcase
    it.upcase # error: Method `upcase` does not exist on `Integer`
  end

  # Back to outer scope, 'it' is Array again
  outer_first = it.first
end

# Nested blocks - 'it' in outer block is Array
[["a", "b"], ["c", "d"]].each do
  # 'it' is the array ["a", "b"] or ["c", "d"]
  x = it.length
end

# Nested blocks with mixed types work correctly
[1, 2, 3].map do
  # Outer 'it' is Integer
  ["a", "b"].map do
    # Inner 'it' is String
    it.upcase + it.downcase
  end
end

# Each level of nesting has its own 'it'
result = [[1, 2], [3, 4]].map do
  # Outer 'it' is an Array
  it.map do
    # Inner 'it' is an Integer
    it * 2
  end
end

# Triple nesting - each 'it' is independent with distinct types
# Level 1 is Array (has .each), Level 2 is String (has .upcase), Level 3 is Integer (has .zero?)
[["hello", "world"], ["foo", "bar"]].each do
  # Outer 'it' is Array[String]
  it.each do
    # Middle 'it' is String
    middle_upper = it.upcase # String has upcase

    it.length.times do
      # Inner 'it' is Integer
      is_zero = it.zero? # Integer has .zero? (String doesn't)
    end
  end
end

# Type error in deeply nested context
[[[1, 2]], [[3, 4]]].each do
  it.each do
    it.each do
      # 'it' is Integer here
      it.upcase # error: Method `upcase` does not exist on `Integer`
    end
  end
end
