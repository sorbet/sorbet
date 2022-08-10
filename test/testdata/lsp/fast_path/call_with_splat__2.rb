# typed: strict
# spacer for exclude-from-file-update

res = A.example(&:foo)
T.reveal_type(res) # error: `String`
res = A.example(&:fooo) # error: Method `fooo` does not exist on `A`
T.reveal_type(res) # error: `T.untyped`

int_str = [1, '']
str_int = ['', 1]
int_str_false = [1, '', false]
  A.takes_int_and_string(*int_str)
# spacer
# spacer
  A.takes_int_and_string(*str_int)
# ^ error: Expected `Integer` but found `String("")` for argument `x`
# ^ error: Expected `String` but found `Integer(1)` for argument `y`
  A.takes_int_and_string(*int_str_false)
# ^ error: Too many arguments provided
# spacer
# spacer
