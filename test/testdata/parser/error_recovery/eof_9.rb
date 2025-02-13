# typed: false

case x # parser-error: Hint: this "case" token is not closed before the end of the file
when true

case # parser-error: Hint: this "case" token is not closed before the end of the file
when x

  case x # parser-error: Hint: this "case" token is not closed before the end of the file
# ^^^^ parser-error: unexpected token "case"

