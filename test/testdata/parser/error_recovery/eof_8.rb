# typed: false
#        ^ to silence the dead code errors

# TODO(jez) We don't do the same rewinding for these constructs that we do for
# class/method blocks, so the indentation levels are not used for the sake of
# backtracking at the moment.

class A # error: Hint: this "class" token is not closed before the end of the file
  def foo(x, y) # error: Hint: this "def" token is not closed before the end of the file
    unless x # error: Hint: this "unless" token is not closed before the end of the file
    while x # error: Hint: this "while" token is not closed before the end of the file
    until x # error: Hint: this "until" token is not closed before the end of the file
    1.times do # error: Hint: this "do" token is not closed before the end of the file
    -> do # error: Hint: this kDO_LAMBDA token is not closed before the end of the file
    x y do # error: Hint: this kDO_BLOCK token is not closed before the end of the file
    # error: unexpected token "end of file"
