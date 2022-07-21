# typed: false

class A
  def # error: Hint: this "def" token might not be followed by a method name
  end
end # error: unexpected token "end of file"
