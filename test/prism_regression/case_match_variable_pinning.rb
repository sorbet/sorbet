# typed: false

lvar = 1

case foo

# "Variable pinning", which matches against the value of that variable
in ^lvar # `lvar` must already exist. Does *not* bind a new variable `lvar`, like `in x` below.
  "Has the same value as the preexisting variable `x`"
in ^@ivar
  "Has the same value as `@ivar`"
in ^@@cvar
  "Has the same value as `@@cvar`"
in ^$global
  "Has the same value as `$global`"

# "Expression pinning", which match the result of the expression
in ^(1 + 2)
  "Has the same value as `1 + 2`"

in x # Binds any value to a new variable `x`.
  "Some other value: #{x}"
end
