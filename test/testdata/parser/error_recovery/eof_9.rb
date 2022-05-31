# typed: false

case x # error: Hint: this "case" token is not closed before the end of the file
when true

case # error: Hint: this "case" token is not closed before the end of the file
when x

  case x # error: Hint: this "case" token is not closed before the end of the file
# ^^^^ error: unexpected token "case"

