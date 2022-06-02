# typed: true

class A # error: Hint: this "class" token is not closed before the end of the file
  def foo(x) # error: Hint: this "def" token is not closed before the end of the file
    if x # error: Hint: this "if" token is not closed before the end of the file
      puts(x) # error: unexpected token "end of file"
