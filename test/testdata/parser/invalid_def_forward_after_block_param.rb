# typed: true

# This parser behaviour is really bizarre, and depends on whether `#invalid` has an empty body or not.
# It seems to replace the entire `def invalid` node with whatever comes after that line.
# If `#invalid` has an empty body, then it would take the next method.

def invalid(&, ...)
  #          ^ error: unexpected token ","
  "The body of the invalid method"
end
