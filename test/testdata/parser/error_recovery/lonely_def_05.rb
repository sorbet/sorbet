# typed: false

class A
  def # parser-error: Hint: this "def" token might not be followed by a method name
  end
end # parser-error: unexpected token "end of file"
