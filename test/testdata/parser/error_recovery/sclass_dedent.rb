# typed: false

class A # error: Hint: this "class" token is not closed before the end of the file
  class << self
    def foo
    end

  def bar
  end
end # error: unexpected token "end of file"
