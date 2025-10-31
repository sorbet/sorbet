# typed: true

# When 'it' is defined as a method, anonymous parameter takes precedence
def it
  "method_value"
end

result = [1, 2, 3].map { it * 2 } # Should use block parameter, not the method
# Result is [2, 4, 6]
