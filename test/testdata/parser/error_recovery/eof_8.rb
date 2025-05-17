# typed: false
#        ^ to silence the dead code errors

# TODO(jez) We don't do the same rewinding for these constructs that we do for
# class/method blocks, so the indentation levels are not used for the sake of
# backtracking at the moment.

class A # parser-error: Hint: this "class" token is not closed before the end of the file
  def foo(x, y) # parser-error: Hint: this "def" token is not closed before the end of the file
    unless x # parser-error: Hint: this "unless" token is not closed before the end of the file
    while x # parser-error: Hint: this "while" token is not closed before the end of the file
    until x # parser-error: Hint: this "until" token is not closed before the end of the file
    1.times do # parser-error: Hint: this "do" token is not closed before the end of the file
    -> do # parser-error: Hint: this kDO_LAMBDA token is not closed before the end of the file
    x y do # parser-error: Hint: this kDO_BLOCK token is not closed before the end of the file
    # parser-error: unexpected token "end of file"
