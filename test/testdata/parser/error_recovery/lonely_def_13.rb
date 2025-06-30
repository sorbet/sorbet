# typed: false

class A
  def self.# parser-error: Hint: this "." token might not be followed by a method name
  end
end # parser-error: unexpected token "end of file"
