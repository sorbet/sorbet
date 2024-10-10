# typed: false

case foo
in [x]      # Variable binding nested in an Array pattern
  "An Array-like thing that only contains #{x}"
in { k: x } # Variable binding nested in a Hash pattern
  "A Hash-like whose key `:k` has value #{x}"

in x
  "Some other value: #{x}"
end
