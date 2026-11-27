# typed: strict
# spacer for exclude-from-file-update

int_arg = [1]
str_arg = ['']
  A.new(*int_arg)
# spacer
  A.new(*str_arg)
# ^ error: Expected `Integer` but found `String("")` for argument `x`
