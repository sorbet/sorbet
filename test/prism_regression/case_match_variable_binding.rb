# typed: false

case foo
in [x] # Variable binding nested in an Array pattern
  "An Array-like thing that only contains #{x}"

in x
  "Some other value: #{x}"
end
