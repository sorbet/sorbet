# typed: true

class A # parser-error: Hint: this "class" token is not closed before the end of the file
  def foo(x) # parser-error: Hint: this "def" token is not closed before the end of the file
    if x # parser-error: Hint: this "if" token is not closed before the end of the file
      puts(x) # parser-error: unexpected token "end of file"
