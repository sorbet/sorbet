# typed: false

class A
  class << self
    def foo
    end # error: unexpected token "end of file"
