# typed: false

lvar = 1

case foo
in [x]      # Variable binding nested in an Array pattern
  "An Array-like thing that only contains #{x}"
in { k: x } # Variable binding nested in a Hash pattern
  "A Hash-like whose key `:k` has value #{x}"

in [[value], *tail] # Array pattern inside an Array pattern
  "An array-like thing that starts with a one-element Array containing #{value}, and ends with #{tail}"
in { k: [value] }   # Array pattern inside a Hash pattern
  "A hash-like whose key `:k` has a one-element Array value containing #{value}"

in [{ k: value }, *tail] # A Hash pattern inside an Array pattern
  "An array-like thing that starts with a one-element Hash containing #{value}, and ends with #{tail}"
in { k: { k2: value } }  # A Hash pattern inside a Hash pattern
  "A hash-like whose key `:k` has a one-element Hash value containing k2: #{value}"

# "Variable pinning", which matches against the value of that variable
in ^lvar # `lvar` must already exist. Does *not* bind a new variable `lvar`, like `in x` below.
  "Has the same value as the preexisting variable `x`"
in ^@ivar
  "Has the same value as `@ivar`"
in ^@@cvar
  "Has the same value as `@@cvar`"
in ^$global
  "Has the same value as `$global`"

in x # Binds any value to a new variable `x`.
  "Some other value: #{x}"
end
