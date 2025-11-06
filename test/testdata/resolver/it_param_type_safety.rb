# typed: true

# Test type safety with 'it' parameter

# Correct usage, 'it' inferred as Integer
result1 = [1, 2, 3].map { it * 2 } # Result is [2, 4, 6]

# Correct usage, 'it' inferred as String
result2 = ["a", "b"].map { it.upcase } # Result is ["A", "B"]

# Type error, wrong method for inferred type
result3 = [1, 2, 3].map { it.upcase } # error: Method `upcase` does not exist on `Integer`

# Type error, wrong operator for inferred type
result4 = ["x", "y"].map { it ** 2 } # error: Method `**` does not exist on `String`

# Shadow 'it' with local variable of different type causes type errors
it = 5
["a", "b"].map { it.upcase } # error: Method `upcase` does not exist on `Integer`
