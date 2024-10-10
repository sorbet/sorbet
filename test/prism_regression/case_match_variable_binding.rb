# typed: false

case foo
in [x]      # Variable binding nested in an Array pattern
  "An Array-like thing that only contains #{x}"
in { k: x } # Variable binding nested in a Hash pattern
  "A Hash-like whose key `:k` has value #{x}"

in [[value], *tail] # Array pattern inside an Array pattern
  "An array-like thing that starts with a one-element Array containing #{value}, and ends with #{tail}"
in { k: [value] }   # Array pattern inside a Hash pattern
  "A hash-like whose key `:k` has a one-element Array value containing #{value}"

in x
  "Some other value: #{x}"
end
