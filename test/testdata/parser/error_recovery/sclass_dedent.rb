# typed: false

class A
  class << self # error: Hint: this "class" token might not be properly closed
    def foo
    end

  def bar
  end
end # error: unexpected token "end of file"
