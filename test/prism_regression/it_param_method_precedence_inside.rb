# typed: false

# When 'it' is defined as a method, anonymous parameter takes precedence
def it
  "method_value"
end

result = [1, 2, 3].map { it * 2 } # Should use block parameter, not the method
# Result is [2, 4, 6]

# Explicit method call when 'it' method doesn't exist on the receiver
tap { self.it }
#          ^^ error: Method `it` does not exist on `T.class_of(<root>)`

# 'it()' with parentheses should call the method, not use the block parameter
tap { it() }
# This calls the 'it' method defined above, returns "method_value"
