# typed: false

class A # error: Hint: this "class" token is not closed before the end of the file
  class << self # error: Hint: this "class" token is not closed before the end of the file
    def foo
    end # error: unexpected token "end of file"
